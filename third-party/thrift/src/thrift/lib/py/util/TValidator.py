# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import logging

from thrift.Thrift import TType

_log = logging.getLogger("thrift.validator")

import sys

if sys.version_info[0] >= 3:
    basestring = str
    unicode = str
    long = int


class TValidator:
    tinfo = {
        # ttype: (type_name, python_type, min_value, max_value)
        TType.BOOL: ("BOOL", bool, None, None),
        TType.BYTE: ("BYTE", int, -128, 127),
        TType.DOUBLE: ("DOUBLE", float, None, None),
        TType.I16: ("I16", int, -32768, 32767),
        TType.I32: ("I32", int, -2147483648, 2147483647),
        TType.I64: ("I64", (int, long), None, None),
        TType.STRING: ("STRING", basestring, None, None),
        TType.UTF8: ("UTF8", unicode, None, None),
    }

    def __init__(self):
        self.custom_validators = {}

    def addClassValidator(self, name, validator):
        self.custom_validators[name] = validator

    def validate(self, msg):
        # WARNING: `thrift_spec` is a deprecated internal field that only exists in thrift-py-deprecated.
        # Usage of this field is unsafe and will break during migration to newer Thrift versions.
        # Please refactor your code to avoid relying on `thrift_spec`.
        # Details: https://www.internalfb.com/wiki/Thrift-Python/Migration_Guide/From_thrift-py-deprecated/#4-2-using-attribute-thri
        if not hasattr(msg, "thrift_spec"):
            _log.error("Not a valid thrift object")
            return False

        name = msg.__class__.__name__
        return self.check_struct(name, msg, msg.thrift_spec)

    def check_basic(self, name, value, thrift_type):
        if thrift_type not in self.tinfo:
            _log.warn(
                "%s Unrecognized thrift type %d. No validation done!", name, thrift_type
            )
            return True

        t_name, python_type, v_min, v_max = self.tinfo[thrift_type]
        if not isinstance(value, python_type):
            error = "Value %s is not a %s" % (str(value), t_name)
        elif (v_min is not None and value < v_min) or (
            v_max is not None and value > v_max
        ):
            error = "Value %s not within %s boundaries" % (str(value), t_name)
        else:
            error = None

        if error is None:
            _log.debug("%s -> %s (type: %s) OK", name, str(value), t_name)
        else:
            _log.error("ERROR: %s is WRONG. %s", name, error)
        return error is None

    def check_map(self, name, value, k_type, k_specs, v_type, v_specs):
        _log.debug("%s - MAP check:", name)
        ok = True
        for k, v in value.items():
            if not self.check_type("%s key" % (name), k, k_type, k_specs):
                ok = False
            if not self.check_type("%s[%s]" % (name, k), v, v_type, v_specs):
                ok = False
        return ok

    def check_listset(self, name, value, v_type, v_specs):
        _log.debug("%s - LIST/SET check:", name)
        ok = True
        for i, v in enumerate(value):
            if not self.check_type("%s[%d]" % (name, i), v, v_type, v_specs):
                ok = False
        return ok

    def check_type(self, name, value, v_type, specs):
        if value is None:
            _log.debug("%s - NOT set", name)
            return True

        if v_type == TType.STRUCT:
            struct_specs = specs[1]
            ok = self.check_struct(name, value, struct_specs)
        elif v_type == TType.MAP:
            k_type = specs[0]
            k_specs = specs[1]
            v_type = specs[2]
            v_specs = specs[3]
            ok = self.check_map(name, value, k_type, k_specs, v_type, v_specs)
        elif v_type in (TType.LIST, TType.SET):
            v_type = specs[0]
            v_specs = specs[1]
            ok = self.check_listset(name, value, v_type, v_specs)
        else:
            ok = self.check_basic(name, value, v_type)

        return ok

    def check_struct(self, name, value, specs):
        _log.debug("%s - STRUCT check:", name)
        if specs is None:
            _log.error("%s - Empty thrift specs, can not be validated", name)
            return False

        ok = True
        for spec in specs:
            if spec is None:
                # Some fields of the struct might be not defined or
                # skipped old fields and their spec will be None
                continue
            f_name = name + "." + str(spec[2])
            f_type = spec[1]
            f_value = getattr(value, spec[2])
            f_specs = spec[3]
            if not self.check_type(f_name, f_value, f_type, f_specs):
                ok = False

        class_name = value.__class__.__name__
        if ok and class_name in self.custom_validators:
            if self.custom_validators[class_name](value):
                _log.debug("%s - Custom validator for class %s OK", name, class_name)
            else:
                _log.error(
                    "%s - Custom validator for class %s failed!", name, class_name
                )
                ok = False
        return ok
