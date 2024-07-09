#include <thrift/lib/thrift/detail/protocol.h>

namespace apache::thrift::protocol::detail {
class ObjectPatchStruct;
class ValuePatchStruct;
} // namespace apache::thrift::protocol::detail

namespace apache::thrift::op::detail {

template <class>
class StructPatchBase;
template <class>
class UnionPatchBase;
template <class>
class StructPatch;
template <class>
class UnionPatch;

template <>
class StructPatch<protocol::detail::ObjectPatchStruct>;

template <>
class UnionPatch<protocol::detail::ValuePatchStruct>;

using ObjectPatch = StructPatch<protocol::detail::ObjectPatchStruct>;
using ValuePatch = UnionPatch<protocol::detail::ValuePatchStruct>;

template <>
class StructPatch<protocol::detail::ObjectPatchStruct> {
 public:
  protocol::detail::ObjectPatchStruct& toThrift();
  const protocol::detail::ObjectPatchStruct& toThrift() const;
  bool empty() const;

  void assign(protocol::detail::Object);
  void clear();
  ValuePatch& patchIfSet(FieldId);
  void ensure(FieldId, protocol::detail::Value);

 private:
  StructPatchBase<protocol::detail::ObjectPatchStruct>* patch_;
};

template <>
class UnionPatch<protocol::detail::ValuePatchStruct> {
 public:
  protocol::detail::ValuePatchStruct& toThrift();
  const protocol::detail::ValuePatchStruct& toThrift() const;
  bool empty() const;

  template <class T>
  auto& as();

 private:
  UnionPatchBase<protocol::detail::ValuePatchStruct>* patch_;
};
} // namespace apache::thrift::op::detail
