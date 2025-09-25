# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2022 Raritan Inc. All rights reserved.

# Minimalistic implementation of the http.client module for Python 2 compatibility

import urllib2

class HTTPConnection:
    def __init__(self, host, timeout):
        self.scheme = "http"
        self.host = host
        self.timeout = timeout
        self.opener = urllib2.OpenerDirector()
        self.opener.add_handler(urllib2.HTTPHandler())
        self.response = None

    def request(self, method, path, body, headers):
        url = "%s://%s%s" % (self.scheme, self.host, path)
        # Suppress Bandit message: URL scheme is known to be "http" or "https".
        request = urllib2.Request(url, data=body, headers=headers) # nosec B310
        self.response = self.opener.open(request, timeout=self.timeout)

    def getresponse(self):
        response = self.response
        self.response = None
        return response

class HTTPSConnection(HTTPConnection):
    def __init__(self, host, timeout, context):
        HTTPConnection.__init__(self, host, timeout)
        self.scheme = "https"
        try:
            self.opener.add_handler(urllib2.HTTPSHandler(context=context))
        except TypeError:
            # Python < 2.7.9
            self.opener.add_handler(urllib2.HTTPSHandler())

class RemoteDisconnected(Exception):
    pass
