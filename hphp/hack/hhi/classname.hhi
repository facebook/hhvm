<?hh

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
  * Creates a classname<mixed> (LazyClass) from input $classname. It does not eagerly
  * verify that the $classname is in fact a valid class or perform modularity
  * checks. Should only be used in rare cases to cast a known string that represents
  * some class to a classname. The preferred method to optain a classname should be using
  * the C::class syntax.
  */
  function classname_from_string_unsafe(string $classname)[]: classname<mixed>;

} // namespace HH
