<?hh

namespace {

interface XHPChild {}

<<__Sealed(StringishObject::class)>>
interface Stringish extends XHPChild {}

interface StringishObject extends Stringish {
  public function __toString(): string;
}

interface IZonedStringishObject extends StringishObject {
  public function __toString()[zoned]: string;
}

interface ILeakSafeStringishObject extends IZonedStringishObject {
  public function __toString()[leak_safe]: string;
}

interface IPureStringishObject extends ILeakSafeStringishObject {
  public function __toString()[]: string;
}

} // namespace

namespace HH {

/**
 * The constant ::class works for types besides classes, including type defs.
 * For any type def TypeDef, TypeDef::class creates a typename<TypeDef>. Unlike
 * classname, typename does not support invoking static method or the new
 * operator, only that it is string representing the name of a type.
 */
newtype typename<+T> as string = string;

/**
 * For any class C, C::class creates a classname<C>. Due to the properties
 * of opaque types, C::class is the only way of obtaining a classname.
 */
newtype classname<+T> as typename<T> = typename<T>;

/**
 * A type representing the runtime union of strings, lazy classes, and class
 * pointers without any logging. The value of this type is overwritten
 * internally. Either a nameof C or a C::class should be able to flow into this
 * type without logging or type errors.
 */
type class_or_classname<+T> = classname<T>;
} // namespace HH
