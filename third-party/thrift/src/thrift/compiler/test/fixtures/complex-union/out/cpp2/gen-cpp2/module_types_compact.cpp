/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/complex-union/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#include "thrift/compiler/test/fixtures/complex-union/gen-cpp2/module_types.tcc"

namespace cpp2 {

template void ComplexUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t ComplexUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t ComplexUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t ComplexUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void ListUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t ListUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t ListUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t ListUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void DataUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t DataUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t DataUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t DataUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void Val::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t Val::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t Val::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t Val::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void ValUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t ValUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t ValUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t ValUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void VirtualComplexUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t VirtualComplexUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t VirtualComplexUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t VirtualComplexUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void NonCopyableStruct::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t NonCopyableStruct::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t NonCopyableStruct::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t NonCopyableStruct::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

template void NonCopyableUnion::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t NonCopyableUnion::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t NonCopyableUnion::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t NonCopyableUnion::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

} // namespace cpp2
