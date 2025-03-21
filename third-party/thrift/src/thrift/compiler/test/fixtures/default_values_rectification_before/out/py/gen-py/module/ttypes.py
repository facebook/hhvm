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

__all__ = ['UTF8STRINGS', 'EmptyStruct', 'TestStruct']

class EmptyStruct:

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
    oprot.writeStructBegin('EmptyStruct')
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def __repr__(self):
    L = []
    padding = ' ' * 4
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
    python_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.thrift_types")
    return thrift.python.converter.to_python_struct(python_types.EmptyStruct, self)

  def _to_mutable_python(self):
    import importlib
    import thrift.python.mutable_converter
    python_mutable_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.thrift_mutable_types")
    return thrift.python.mutable_converter.to_mutable_python_struct_or_union(python_mutable_types.EmptyStruct, self)

  def _to_py3(self):
    import importlib
    import thrift.py3.converter
    py3_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.types")
    return thrift.py3.converter.to_py3_struct(py3_types.EmptyStruct, self)

  def _to_py_deprecated(self):
    return self

class TestStruct:
  r"""
  Attributes:
   - unqualified_int_field
   - unqualified_bool_field
   - unqualified_list_field
   - unqualified_struct_field
   - optional_int_field
   - optional_bool_field
   - optional_list_field
   - optional_struct_field
  """

  thrift_spec = None
  thrift_field_annotations = None
  thrift_struct_annotations = None
  __init__ = None
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
      if fid == 1:
        if ftype == TType.I32:
          self.unqualified_int_field = iprot.readI32()
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.BOOL:
          self.unqualified_bool_field = iprot.readBool()
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.LIST:
          self.unqualified_list_field = []
          (_etype3, _size0) = iprot.readListBegin()
          if _size0 >= 0:
            for _i4 in range(_size0):
              _elem5 = iprot.readI32()
              self.unqualified_list_field.append(_elem5)
          else: 
            while iprot.peekList():
              _elem6 = iprot.readI32()
              self.unqualified_list_field.append(_elem6)
          iprot.readListEnd()
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.STRUCT:
          self.unqualified_struct_field = EmptyStruct()
          self.unqualified_struct_field.read(iprot)
        else:
          iprot.skip(ftype)
      elif fid == 5:
        if ftype == TType.I32:
          self.optional_int_field = iprot.readI32()
        else:
          iprot.skip(ftype)
      elif fid == 6:
        if ftype == TType.BOOL:
          self.optional_bool_field = iprot.readBool()
        else:
          iprot.skip(ftype)
      elif fid == 7:
        if ftype == TType.LIST:
          self.optional_list_field = []
          (_etype10, _size7) = iprot.readListBegin()
          if _size7 >= 0:
            for _i11 in range(_size7):
              _elem12 = iprot.readI32()
              self.optional_list_field.append(_elem12)
          else: 
            while iprot.peekList():
              _elem13 = iprot.readI32()
              self.optional_list_field.append(_elem13)
          iprot.readListEnd()
        else:
          iprot.skip(ftype)
      elif fid == 8:
        if ftype == TType.STRUCT:
          self.optional_struct_field = EmptyStruct()
          self.optional_struct_field.read(iprot)
        else:
          iprot.skip(ftype)
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
    oprot.writeStructBegin('TestStruct')
    if self.unqualified_int_field != None:
      oprot.writeFieldBegin('unqualified_int_field', TType.I32, 1)
      oprot.writeI32(self.unqualified_int_field)
      oprot.writeFieldEnd()
    if self.unqualified_bool_field != None:
      oprot.writeFieldBegin('unqualified_bool_field', TType.BOOL, 2)
      oprot.writeBool(self.unqualified_bool_field)
      oprot.writeFieldEnd()
    if self.unqualified_list_field != None:
      oprot.writeFieldBegin('unqualified_list_field', TType.LIST, 3)
      oprot.writeListBegin(TType.I32, len(self.unqualified_list_field))
      for iter14 in self.unqualified_list_field:
        oprot.writeI32(iter14)
      oprot.writeListEnd()
      oprot.writeFieldEnd()
    if self.unqualified_struct_field != None:
      oprot.writeFieldBegin('unqualified_struct_field', TType.STRUCT, 4)
      self.unqualified_struct_field.write(oprot)
      oprot.writeFieldEnd()
    if self.optional_int_field != None and self.optional_int_field != self.thrift_spec[5][4]:
      oprot.writeFieldBegin('optional_int_field', TType.I32, 5)
      oprot.writeI32(self.optional_int_field)
      oprot.writeFieldEnd()
    if self.optional_bool_field != None and self.optional_bool_field != self.thrift_spec[6][4]:
      oprot.writeFieldBegin('optional_bool_field', TType.BOOL, 6)
      oprot.writeBool(self.optional_bool_field)
      oprot.writeFieldEnd()
    if self.optional_list_field != None and self.optional_list_field != self.thrift_spec[7][4]:
      oprot.writeFieldBegin('optional_list_field', TType.LIST, 7)
      oprot.writeListBegin(TType.I32, len(self.optional_list_field))
      for iter15 in self.optional_list_field:
        oprot.writeI32(iter15)
      oprot.writeListEnd()
      oprot.writeFieldEnd()
    if self.optional_struct_field != None and self.optional_struct_field != self.thrift_spec[8][4]:
      oprot.writeFieldBegin('optional_struct_field', TType.STRUCT, 8)
      self.optional_struct_field.write(oprot)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def __repr__(self):
    L = []
    padding = ' ' * 4
    if self.unqualified_int_field is not None:
      value = pprint.pformat(self.unqualified_int_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    unqualified_int_field=%s' % (value))
    if self.unqualified_bool_field is not None:
      value = pprint.pformat(self.unqualified_bool_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    unqualified_bool_field=%s' % (value))
    if self.unqualified_list_field is not None:
      value = pprint.pformat(self.unqualified_list_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    unqualified_list_field=%s' % (value))
    if self.unqualified_struct_field is not None:
      value = pprint.pformat(self.unqualified_struct_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    unqualified_struct_field=%s' % (value))
    if self.optional_int_field is not None:
      value = pprint.pformat(self.optional_int_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    optional_int_field=%s' % (value))
    if self.optional_bool_field is not None:
      value = pprint.pformat(self.optional_bool_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    optional_bool_field=%s' % (value))
    if self.optional_list_field is not None:
      value = pprint.pformat(self.optional_list_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    optional_list_field=%s' % (value))
    if self.optional_struct_field is not None:
      value = pprint.pformat(self.optional_struct_field, indent=0)
      value = padding.join(value.splitlines(True))
      L.append('    optional_struct_field=%s' % (value))
    return "%s(%s)" % (self.__class__.__name__, "\n" + ",\n".join(L) if L else '')

  def __eq__(self, other):
    if not isinstance(other, self.__class__):
      return False

    return self.__dict__ == other.__dict__ 

  def __ne__(self, other):
    return not (self == other)

  def __dir__(self):
    return (
      'unqualified_int_field',
      'unqualified_bool_field',
      'unqualified_list_field',
      'unqualified_struct_field',
      'optional_int_field',
      'optional_bool_field',
      'optional_list_field',
      'optional_struct_field',
    )

  __hash__ = object.__hash__

  def _to_python(self):
    import importlib
    import thrift.python.converter
    python_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.thrift_types")
    return thrift.python.converter.to_python_struct(python_types.TestStruct, self)

  def _to_mutable_python(self):
    import importlib
    import thrift.python.mutable_converter
    python_mutable_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.thrift_mutable_types")
    return thrift.python.mutable_converter.to_mutable_python_struct_or_union(python_mutable_types.TestStruct, self)

  def _to_py3(self):
    import importlib
    import thrift.py3.converter
    py3_types = importlib.import_module("facebook.thrift.compiler.test.fixtures.default_values_rectification.module.types")
    return thrift.py3.converter.to_py3_struct(py3_types.TestStruct, self)

  def _to_py_deprecated(self):
    return self

all_structs.append(EmptyStruct)
EmptyStruct.thrift_spec = tuple(__EXPAND_THRIFT_SPEC((
)))

EmptyStruct.thrift_struct_annotations = {
}
EmptyStruct.thrift_field_annotations = {
}

all_structs.append(TestStruct)
TestStruct.thrift_spec = tuple(__EXPAND_THRIFT_SPEC((
  (1, TType.I32, 'unqualified_int_field', None, 0, 2, ), # 1
  (2, TType.BOOL, 'unqualified_bool_field', None, False, 2, ), # 2
  (3, TType.LIST, 'unqualified_list_field', (TType.I32,None), [
  ], 2, ), # 3
  (4, TType.STRUCT, 'unqualified_struct_field', [EmptyStruct, EmptyStruct.thrift_spec, False], EmptyStruct(**{
  }), 2, ), # 4
  (5, TType.I32, 'optional_int_field', None, 42, 1, ), # 5
  (6, TType.BOOL, 'optional_bool_field', None, True, 1, ), # 6
  (7, TType.LIST, 'optional_list_field', (TType.I32,None), [
    1,
    2,
  ], 1, ), # 7
  (8, TType.STRUCT, 'optional_struct_field', [EmptyStruct, EmptyStruct.thrift_spec, False], EmptyStruct(**{
  }), 1, ), # 8
)))

TestStruct.thrift_struct_annotations = {
}
TestStruct.thrift_field_annotations = {
}

def TestStruct__init__(self, unqualified_int_field=TestStruct.thrift_spec[1][4], unqualified_bool_field=TestStruct.thrift_spec[2][4], unqualified_list_field=TestStruct.thrift_spec[3][4], unqualified_struct_field=TestStruct.thrift_spec[4][4], optional_int_field=TestStruct.thrift_spec[5][4], optional_bool_field=TestStruct.thrift_spec[6][4], optional_list_field=TestStruct.thrift_spec[7][4], optional_struct_field=TestStruct.thrift_spec[8][4],):
  self.unqualified_int_field = unqualified_int_field
  self.unqualified_bool_field = unqualified_bool_field
  if unqualified_list_field is self.thrift_spec[3][4]:
    unqualified_list_field = [
  ]
  self.unqualified_list_field = unqualified_list_field
  if unqualified_struct_field is self.thrift_spec[4][4]:
    unqualified_struct_field = EmptyStruct(**{
  })
  self.unqualified_struct_field = unqualified_struct_field
  self.optional_int_field = optional_int_field
  self.optional_bool_field = optional_bool_field
  if optional_list_field is self.thrift_spec[7][4]:
    optional_list_field = [
    1,
    2,
  ]
  self.optional_list_field = optional_list_field
  if optional_struct_field is self.thrift_spec[8][4]:
    optional_struct_field = EmptyStruct(**{
  })
  self.optional_struct_field = optional_struct_field

TestStruct.__init__ = TestStruct__init__

def TestStruct__setstate__(self, state):
  state.setdefault('unqualified_int_field', 0)
  state.setdefault('unqualified_bool_field', False)
  state.setdefault('unqualified_list_field', [
  ])
  state.setdefault('unqualified_struct_field', EmptyStruct(**{
  }))
  state.setdefault('optional_int_field', 42)
  state.setdefault('optional_bool_field', True)
  state.setdefault('optional_list_field', [
    1,
    2,
  ])
  state.setdefault('optional_struct_field', EmptyStruct(**{
  }))
  self.__dict__ = state

TestStruct.__getstate__ = lambda self: self.__dict__.copy()
TestStruct.__setstate__ = TestStruct__setstate__

fix_spec(all_structs)
del all_structs
