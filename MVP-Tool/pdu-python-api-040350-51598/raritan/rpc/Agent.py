# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2010 Raritan Inc. All rights reserved.

# Avoid name clash with raritan.rpc.sys
from __future__ import absolute_import

import base64, json, ssl, sys, uuid
import raritan.rpc

try:
    # Python 3
    import http.client as http_client
    from urllib.parse import quote, urlparse
except ImportError:
    # Python 2
    from raritan.rpc import legacy_http_client as http_client
    from urlparse import urlparse
    from urllib import quote

class Agent(object):
    """Provides transport to one RPC service, e.g. one PX2 device - holds host,
       user name, and password."""
    id = 0

    def __init__(self, proto, host, user = None, passwd = None, token = None,
                 debug = False, disable_certificate_verification = False, timeout = None):
        self._scheme = proto
        self._host = host
        self.user = user
        self.passwd = passwd
        self.token = token # authentication token
        self.debug = debug
        self.timeout = timeout
        self._connection = None
        self._context = None
        if disable_certificate_verification:
            if "_create_unverified_context" in ssl.__dict__.keys():
                # Suppress Bandit message: Only done on explicit request by the user, default
                # behavior is to verify the certificate.
                self._context = ssl._create_unverified_context() # nosec B323
    @property
    def url(self):
        return self._scheme + "://" + self._host

    def set_auth_basic(self, user, passwd):
        self.user = user
        self.passwd = passwd
        self.token = None

    def set_auth_token(self, token):
        self.user = None
        self.passwd = None
        self.token = token

    def _request(self, method, path, body, headers, redirected=False):
        if self._connection and self._connection.timeout != self.timeout:
            # force a new connection in case the timeout has been changed
            self._connection = None

        have_connection = bool(self._connection)
        if not have_connection:
            if self._scheme == "http":
                self._connection = http_client.HTTPConnection(self._host, timeout=self.timeout)
            elif self._scheme == "https":
                self._connection = http_client.HTTPSConnection(self._host, context=self._context, timeout=self.timeout)
            else:
                raise ValueError("Unsupported scheme: " + self._scheme)

        if not path.startswith("/"):
            path = "/" + path

        try:
            self._connection.request(method, path, body, headers)
            response = self._connection.getresponse()
        except (ssl.SSLEOFError, BrokenPipeError, ConnectionResetError):
            if have_connection:
                # connection was closed by the server; retry the request
                self._connection = None
                return self._request(method, path, body, headers)
            raise
        except IOError as e:
            if str(e).find("CERTIFICATE_VERIFY_FAILED") >= 0:
                sys.stderr.write("==================================================================\n")
                sys.stderr.write(" SSL certificate verification failed!\n")
                sys.stderr.write("\n")
                sys.stderr.write(" When connecting to a device without valid SSL certificate, try\n")
                sys.stderr.write(" adding 'disable_certificate_verification=True' when creating the\n")
                sys.stderr.write(" raritan.rpc.Agent instance.\n")
                sys.stderr.write("==================================================================\n")
            raise

        if have_connection and response.code == 408:
            # HTTP connection timed out on the server; retry the request
            self._connection = None
            return self._request(method, path, body, headers)

        if response.code in [ 302, 307 ] and not redirected:
            # handle HTTP-to-HTTPS redirect and try again
            if self._handle_http_redirect(path, response):
                return self._request(method, path, body, headers, True)

        return response

    def _get_auth_headers(self):
        headers = {}
        if self.token != None:
            headers['X-SessionToken'] = self.token
        elif self.user != None and self.passwd != None:
            basic = base64.b64encode(str.encode('%s:%s' % (self.user, self.passwd)))
            headers['Authorization'] = 'Basic ' + bytes.decode(basic)
        return headers

    def _handle_http_redirect(self, rid, response):
        new_url = urlparse(response.headers["Location"])
        if self.debug:
            print("Redirected to: %s://%s" % (new_url.scheme, new_url.netloc))
        self._scheme = new_url.scheme
        self._host = new_url.netloc
        self._connection = None
        return True

    def get(self, target):
        try:
            response = self._request('GET', target, None, self._get_auth_headers())
            resp = response.read()
        except Exception as e:
            self._connection = None
            raise raritan.rpc.HttpException("HTTP request failed", e)

        if response.code != 200:
            raise raritan.rpc.HttpException("HTTP Error %d\nResponse:\n%s" % (response.code, str(resp)))

        if self.debug:
            print("download: Response:\n%s" % str(resp))

        return resp

    def form_data_file(self, target, datas):
        boundary = uuid.uuid4().hex
        bodyArr = []
        for data in datas:
            filedata = data['data']
            filename = data['filename']
            formname = data['formname']
            mimetype = data['mimetype']
            bodyArr.append('--%s' % boundary)
            bodyArr.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (formname, filename))
            bodyArr.append('Content-Type: %s' % mimetype)
            bodyArr.append('')
            bodyArr.append(filedata)
        bodyArr.append('--%s--' % boundary)
        body = bytes()
        for l in bodyArr:
            if isinstance(l, bytes): body += l + b'\r\n'
            else: body += bytes(l, encoding='utf8') + b'\r\n'

        headers = { 'Content-Type': 'multipart/form-data; boundary=' + boundary }
        headers.update(self._get_auth_headers())

        try:
            response = self._request('POST', target, body, headers)
            resp = bytes.decode(response.read())
        except Exception as e:
            self._connection = None
            raise raritan.rpc.HttpException("HTTP request failed", e)

        if response.code != 200:
            raise raritan.rpc.HttpException("HTTP Error %d\nResponse:\n%s" % (response.code, resp))

        if self.debug:
            print("form_data: Response:\n%s" % resp)

        # can't return the response object, because the read operation can only called once
        # (HTTPResponse Objects are not seekable)
        # https://docs.python.org/3/library/http.client.html#httpresponse-objects
        return dict(
            headers = response.headers,
            body = resp
        )

    def json_rpc(self, target, method, params = []):
        Agent.id += 1
        request_json = json.dumps({"method": method, "params": params, "id": Agent.id})
        if self.debug:
            print("json_rpc: %s() - %s: , request = %s" % (method, target, request_json))

        headers = { 'Content-Type': 'application/json; charset=UTF-8' }
        headers.update(self._get_auth_headers())

        try:
            response = self._request('POST', quote(target), request_json, headers)
            resp = bytes.decode(response.read(), errors='replace')
        except Exception as e:
            self._connection = None
            raise raritan.rpc.HttpException("HTTP request failed.", e)

        if response.code != 200:
            raise raritan.rpc.HttpException("HTTP Error %d\nResponse:\n%s" % (response.code, resp))

        if self.debug:
            print("json_rpc: Response:\n%s" % resp)

        try:
            resp_json = json.loads(resp)
        except ValueError as e:
            raise raritan.rpc.JsonRpcSyntaxException(
                    "Decoding response to JSON failed: %s" % e)

        if "error" in resp_json:
            try:
                code = resp_json["error"]["code"]
                msg = resp_json["error"]["message"]
            except KeyError:
                raise raritan.rpc.JsonRpcSyntaxException(
                        "JSON RPC returned malformed error: %s" % resp_json)
            raise raritan.rpc.JsonRpcErrorException(
                    "JSON RPC returned error: code = %d, msg = %s" % (code, msg))

        try:
            res = resp_json["result"]
        except KeyError:
            raise raritan.rpc.JsonRpcSyntaxException(
                    "Result is missing in JSON RPC response: %s" % resp_json)

        return res
