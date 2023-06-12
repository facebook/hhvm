<?hh

interface Visibility {}
final class CPublic implements Visibility {}
final class CProtected implements Visibility {}

class Field<+TShape, +TValue> {}

class X {}

enum class E : mixed {
  Field<shape('access' => CPublic), int> EPub = new Field();
  Field<shape('access' => CProtected), string> EProt = new Field();
}

function getProtectedField<
  TValue,
  TField as Field<
    shape('access' => CProtected, ...),
    TValue,
    >,
  >(HH\EnumClass\Label<E, TField> $label): TValue {
  invariant_violation('yolo');
}

function main(): string {
  return getProtectedField(#AUTO332
}
