#
# Autogenerated by Thrift for thrift/compiler/test/fixtures/py3/src/module.thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

import enum
import thrift.py3.types
import module.thrift_enums as _fbthrift_python_enums

_fbthrift__module_name__ = "module.types"



class AnEnum(thrift.py3.types.CompiledEnum, int):
    NOTSET = 0
    ONE = 1
    TWO = 2
    THREE = 3
    FOUR = 4

    __module__ = _fbthrift__module_name__
    __slots__ = ()

    @staticmethod
    def __get_metadata__():
        return _fbthrift_python_enums.gen_metadata_enum_AnEnum()

    @staticmethod
    def __get_thrift_name__():
        return "module.AnEnum"

    def _to_python(self):
        return _fbthrift_python_enums.AnEnum(self._fbthrift_value_)

    def _to_py3(self):
        return self

    def _to_py_deprecated(self):
        return self._fbthrift_value_


    def __int__(self):
        return self._fbthrift_value_

    def __index__(self):
        return self._fbthrift_value_

class AnEnumRenamed(thrift.py3.types.CompiledEnum, int):
    name_ = 0
    value_ = 1
    renamed_ = 2

    __module__ = _fbthrift__module_name__
    __slots__ = ()

    @staticmethod
    def __get_metadata__():
        return _fbthrift_python_enums.gen_metadata_enum_AnEnumRenamed()

    @staticmethod
    def __get_thrift_name__():
        return "module.AnEnumRenamed"

    def _to_python(self):
        return _fbthrift_python_enums.AnEnumRenamed(self._fbthrift_value_)

    def _to_py3(self):
        return self

    def _to_py_deprecated(self):
        return self._fbthrift_value_


    def __int__(self):
        return self._fbthrift_value_

    def __index__(self):
        return self._fbthrift_value_

class Flags(thrift.py3.types.Flag):
    flag_A = 1
    flag_B = 2
    flag_C = 4
    flag_D = 8

    __module__ = _fbthrift__module_name__
    __slots__ = ()

    @staticmethod
    def __get_metadata__():
        return _fbthrift_python_enums.gen_metadata_enum_Flags()

    @staticmethod
    def __get_thrift_name__():
        return "module.Flags"

    def _to_python(self):
        return _fbthrift_python_enums.Flags(self._fbthrift_value_)

    def _to_py3(self):
        return self

    def _to_py_deprecated(self):
        return self._fbthrift_value_


    def __int__(self):
        return self._fbthrift_value_

    def __index__(self):
        return self._fbthrift_value_


class __BinaryUnionType(enum.Enum):
    iobuf_val = 1
    EMPTY = 0

    __module__ = _fbthrift__module_name__
    __slots__ = ()
