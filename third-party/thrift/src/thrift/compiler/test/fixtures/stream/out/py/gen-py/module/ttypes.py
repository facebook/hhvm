#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from __future__ import absolute_import
import sys
from thrift.util.Recursive import fix_spec
from thrift.Thrift import TType, TMessageType, TPriority, TRequestContext, TProcessorEventHandler, TServerInterface, TProcessor, TException, TApplicationException, UnimplementedTypedef
from thrift.protocol.TProtocol import TProtocolException

from json import loads
import sys
if sys.version_info[0] >= 3:
  long = int


import pprint
import warnings
from thrift import Thrift
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TCompactProtocol
from thrift.protocol import THeaderProtocol
fastproto = None
try:
  from thrift.protocol import fastproto
except ImportError:
  pass

def __EXPAND_THRIFT_SPEC(spec):
    next_id = 0
    for item in spec:
        item_id = item[0]
        if next_id >= 0 and item_id < 0:
            next_id = item_id
        if item_id != next_id:
            for _ in range(next_id, item_id):
                yield None
        yield item
        next_id = item_id + 1

class ThriftEnumWrapper(int):
  def __new__(cls, enum_class, value):
    return super().__new__(cls, value)
  def __init__(self, enum_class, value):    self.enum_class = enum_class
  def __repr__(self):
    return self.enum_class.__name__ + '.' + self.enum_class._VALUES_TO_NAMES[self]

all_structs = []
UTF8STRINGS = bool(0) or sys.version_info.major >= 3

__all__ = ['UTF8STRINGS', 'FooStreamEx', 'FooEx', 'FooEx2']

class FooStreamEx(TException):

  thrift_spec = None
  thrift_field_annotations = None
  thrift_struct_annotations = None
  @staticmethod
  def isUnion():
    return False

  def read(self, iprot):
    if (isinstance(iprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0)
      return
    if (isinstance(iprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2)
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if (isinstance(oprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0))
      return
    if (isinstance(oprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2))
      return
    oprot.writeStructBegin('FooStreamEx')
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def readFromJson(self, json, is_text=True, **kwargs):
    kwargs_copy = dict(kwargs)
    wrap_enum_constants = kwargs_copy.pop('wrap_enum_constants', False)
    relax_enum_validation = bool(kwargs_copy.pop('relax_enum_validation', not wrap_enum_constants))
    set_cls = kwargs_copy.pop('custom_set_cls', set)
    dict_cls = kwargs_copy.pop('custom_dict_cls', dict)
    if wrap_enum_constants and relax_enum_validation:
        raise ValueError(
            'wrap_enum_constants cannot be used together with relax_enum_validation'
        )
    if kwargs_copy:
        extra_kwargs = ', '.join(kwargs_copy.keys())
        raise ValueError(
            'Unexpected keyword arguments: ' + extra_kwargs
        )
    json_obj = json
    if is_text:
      json_obj = loads(json)

  def __str__(self):
    return repr(self)

  def __repr__(self):
    L = []
    padding = ' ' * 4
    if 'message' not in self.__dict__:
      message = getattr(self, 'message', None)
      if message:
        L.append('message=%r' % message)
    return "%s(%s)" % (self.__class__.__name__, "\n" + ",\n".join(L) if L else '')

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False

    return self.__dict__ == other.__dict__ 

  def __ne__(self, other):
    return not (self == other)

  def __dir__(self):
    return (
    )

  __hash__ = object.__hash__

  def _to_python(self):
    import importlib
    import thrift.python.converter
    python_types = importlib.import_module("module.thrift_types")
    return thrift.python.converter.to_python_struct(python_types.FooStreamEx, self)

  def _to_mutable_python(self):
    import importlib
    import thrift.python.mutable_converter
    python_mutable_types = importlib.import_module("module.thrift_mutable_types")
    return thrift.python.mutable_converter.to_mutable_python_struct_or_union(python_mutable_types.FooStreamEx, self)

  def _to_py3(self):
    import importlib
    import thrift.py3.converter
    py3_types = importlib.import_module("module.types")
    return thrift.py3.converter.to_py3_struct(py3_types.FooStreamEx, self)

  def _to_py_deprecated(self):
    return self

class FooEx(TException):

  thrift_spec = None
  thrift_field_annotations = None
  thrift_struct_annotations = None
  @staticmethod
  def isUnion():
    return False

  def read(self, iprot):
    if (isinstance(iprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0)
      return
    if (isinstance(iprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2)
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if (isinstance(oprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0))
      return
    if (isinstance(oprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2))
      return
    oprot.writeStructBegin('FooEx')
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def readFromJson(self, json, is_text=True, **kwargs):
    kwargs_copy = dict(kwargs)
    wrap_enum_constants = kwargs_copy.pop('wrap_enum_constants', False)
    relax_enum_validation = bool(kwargs_copy.pop('relax_enum_validation', not wrap_enum_constants))
    set_cls = kwargs_copy.pop('custom_set_cls', set)
    dict_cls = kwargs_copy.pop('custom_dict_cls', dict)
    if wrap_enum_constants and relax_enum_validation:
        raise ValueError(
            'wrap_enum_constants cannot be used together with relax_enum_validation'
        )
    if kwargs_copy:
        extra_kwargs = ', '.join(kwargs_copy.keys())
        raise ValueError(
            'Unexpected keyword arguments: ' + extra_kwargs
        )
    json_obj = json
    if is_text:
      json_obj = loads(json)

  def __str__(self):
    return repr(self)

  def __repr__(self):
    L = []
    padding = ' ' * 4
    if 'message' not in self.__dict__:
      message = getattr(self, 'message', None)
      if message:
        L.append('message=%r' % message)
    return "%s(%s)" % (self.__class__.__name__, "\n" + ",\n".join(L) if L else '')

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False

    return self.__dict__ == other.__dict__ 

  def __ne__(self, other):
    return not (self == other)

  def __dir__(self):
    return (
    )

  __hash__ = object.__hash__

  def _to_python(self):
    import importlib
    import thrift.python.converter
    python_types = importlib.import_module("module.thrift_types")
    return thrift.python.converter.to_python_struct(python_types.FooEx, self)

  def _to_mutable_python(self):
    import importlib
    import thrift.python.mutable_converter
    python_mutable_types = importlib.import_module("module.thrift_mutable_types")
    return thrift.python.mutable_converter.to_mutable_python_struct_or_union(python_mutable_types.FooEx, self)

  def _to_py3(self):
    import importlib
    import thrift.py3.converter
    py3_types = importlib.import_module("module.types")
    return thrift.py3.converter.to_py3_struct(py3_types.FooEx, self)

  def _to_py_deprecated(self):
    return self

class FooEx2(TException):

  thrift_spec = None
  thrift_field_annotations = None
  thrift_struct_annotations = None
  @staticmethod
  def isUnion():
    return False

  def read(self, iprot):
    if (isinstance(iprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0)
      return
    if (isinstance(iprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(iprot, THeaderProtocol.THeaderProtocolAccelerate) and iprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastproto is not None:
      fastproto.decode(self, iprot.trans, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2)
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if (isinstance(oprot, TBinaryProtocol.TBinaryProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=0))
      return
    if (isinstance(oprot, TCompactProtocol.TCompactProtocolAccelerated) or (isinstance(oprot, THeaderProtocol.THeaderProtocolAccelerate) and oprot.get_protocol_id() == THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL)) and self.thrift_spec is not None and fastproto is not None:
      oprot.trans.write(fastproto.encode(self, [self.__class__, self.thrift_spec, False], utf8strings=UTF8STRINGS, protoid=2))
      return
    oprot.writeStructBegin('FooEx2')
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def readFromJson(self, json, is_text=True, **kwargs):
    kwargs_copy = dict(kwargs)
    wrap_enum_constants = kwargs_copy.pop('wrap_enum_constants', False)
    relax_enum_validation = bool(kwargs_copy.pop('relax_enum_validation', not wrap_enum_constants))
    set_cls = kwargs_copy.pop('custom_set_cls', set)
    dict_cls = kwargs_copy.pop('custom_dict_cls', dict)
    if wrap_enum_constants and relax_enum_validation:
        raise ValueError(
            'wrap_enum_constants cannot be used together with relax_enum_validation'
        )
    if kwargs_copy:
        extra_kwargs = ', '.join(kwargs_copy.keys())
        raise ValueError(
            'Unexpected keyword arguments: ' + extra_kwargs
        )
    json_obj = json
    if is_text:
      json_obj = loads(json)

  def __str__(self):
    return repr(self)

  def __repr__(self):
    L = []
    padding = ' ' * 4
    if 'message' not in self.__dict__:
      message = getattr(self, 'message', None)
      if message:
        L.append('message=%r' % message)
    return "%s(%s)" % (self.__class__.__name__, "\n" + ",\n".join(L) if L else '')

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False

    return self.__dict__ == other.__dict__ 

  def __ne__(self, other):
    return not (self == other)

  def __dir__(self):
    return (
    )

  __hash__ = object.__hash__

  def _to_python(self):
    import importlib
    import thrift.python.converter
    python_types = importlib.import_module("module.thrift_types")
    return thrift.python.converter.to_python_struct(python_types.FooEx2, self)

  def _to_mutable_python(self):
    import importlib
    import thrift.python.mutable_converter
    python_mutable_types = importlib.import_module("module.thrift_mutable_types")
    return thrift.python.mutable_converter.to_mutable_python_struct_or_union(python_mutable_types.FooEx2, self)

  def _to_py3(self):
    import importlib
    import thrift.py3.converter
    py3_types = importlib.import_module("module.types")
    return thrift.py3.converter.to_py3_struct(py3_types.FooEx2, self)

  def _to_py_deprecated(self):
    return self

all_structs.append(FooStreamEx)
FooStreamEx.thrift_spec = tuple(__EXPAND_THRIFT_SPEC((
)))

FooStreamEx.thrift_struct_annotations = {
}
FooStreamEx.thrift_field_annotations = {
}

all_structs.append(FooEx)
FooEx.thrift_spec = tuple(__EXPAND_THRIFT_SPEC((
)))

FooEx.thrift_struct_annotations = {
}
FooEx.thrift_field_annotations = {
}

all_structs.append(FooEx2)
FooEx2.thrift_spec = tuple(__EXPAND_THRIFT_SPEC((
)))

FooEx2.thrift_struct_annotations = {
}
FooEx2.thrift_field_annotations = {
}

fix_spec(all_structs)
del all_structs
