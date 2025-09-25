# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2010 Raritan Inc. All rights reserved.

#
# Decodes IDL "typecode" to python class and vice versa.
#

import re
import importlib
import functools

# TODO: generate prefix from "base-package" in config
prefix = "raritan.rpc"

class TypeInfo(object):

    @staticmethod
    def typeBaseName(typeId):
        b = typeId.split(":")[0] # remove version
        b = re.sub(r'_[0-9]*_[0-9]*_[0-9]*', r'', b) # remove version
        return b

    @classmethod
    def idlTypeIdToPyClass(cls, typeId):
        """Returns python class for given typeId.

        The module defining this class will be auto-imported.
        """
        pyName = "%s.%s" % (prefix, TypeInfo.typeBaseName(typeId))
        comps = pyName.split(".")
        className = [comps.pop()]
        module = None
        # remove components from end until import succeeds, import must be inside raritan.rpc
        while len(comps) >= 2:
            modName = ".".join(comps)
            try:
                module = importlib.import_module(modName)
            except ImportError:
                className.insert(0, comps.pop())
                continue
            cls = functools.reduce(getattr, className, module)
            return cls
        raise ImportError("Unable to find package for %s." % typeId)

    @classmethod
    def pyClassToIdlName(cls, pyClass):
        return pyClass.idlType

    @classmethod
    def decode(cls, json):
        typeId = json
        return cls.idlTypeIdToPyClass(typeId)

    @classmethod
    def encode(cls, pyClass):
        return cls.pyClassToIdlName(pyClass)
