# Mixins

<!-- https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/IDL/Mixins/?noredirect -->

## Motivation

The idea is to achieve reusability of common parts of a struct via composition with an additional syntactic sugar.

## Description

Mixins allow exposing functionality of one structure into the namespace of another structure. They look like inheritance without the ambiguity that multiple inheritance can cause. In addition, data schema over the wire is unchanged, thus if `A` has `mixin` field `B`, you can not write `A` to server that accepts `B`.

## Thrift IDL

Basically if you add `mixin` qualifier to thrift field, you will have the ability to access inner fields directly from current struct. For example, if you have following Thrift structs

```
struct Mixin1 { 1: i32 field1; }
struct Mixin2 { 1: i32 field2; }
struct Foo {
  @thrift.Mixin
  1: Mixin1 m1;
  @thrift.Mixin
  2: Mixin2 m2;
  3: i32 field3;
}
```
Then in C++ we could access fields in Mixin from Foo

```
Foo f;
f.field1_ref();  // Access field1 in Mixin1
```
## Codegen

For the example above, we generate following C++ code for Foo struct (simplified):

```
class Foo {
 public:
  field_ref<Mixin1&> m1_ref();
  field_ref<Mixin2&> m2_ref();
  field_ref<int32_t&> field3_ref();

  field_ref<int32_t&> field1_ref() { return m1.field1_ref(); }
  field_ref<int32_t&> field2_ref() { return m2.field2_ref(); }
 private:
  Mixin1 m1;
  Mixin2 m2;
  int32_t field3;
};
```
The highlighted methods are imported from mixin fields `m1` and `m2`.

## Status

Currently it has full support in C++, and readonly support in thrift-py3.

We do not have timeline to support this in other languages. However, if you have strong use case, feel free to let us know by commenting on this [post](https://fb.workplace.com/groups/1730279463893632/permalink/2608655872722649/) to help prioritization.

## FAQ

> Q: Is nested mixin struct supported?

A: Yes. If mixin field itself contains mixin field, all nested fields will be exposed to the top level.

> Q: Can mixin field have optional qualifier?

A: No.

> Q: What if we have duplicated field name in 2 mixin fields?

A: There will be build error with diagnostic message.

Design doc: [here](https://fb.quip.com/Q0QmAQPtJadN).
