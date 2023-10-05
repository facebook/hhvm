<?hh

namespace HH {

  /**
   * The constant ::class works for types besides classes, including type defs.
   * For any type def TypeDef, TypeDef::class creates a typename<TypeDef>. Unlike
   * classname, typename does not support invoking static method or the new
   * operator, only that it is string representing the name of a type.
   */
  <<__NoAutoDynamic>>
  newtype typename<+T> as string = string;

  /**
   * For any class C, C::class creates a classname<C>. Due to the properties
   * of opaque types, C::class is the only way of obtaining a classname.
   */
  <<__NoAutoDynamic>>
  newtype classname<+T> as typename<T> = typename<T>;

  /**
  * Creates a classname<mixed> (LazyClass) from input $classname. It does not eagerly
  * verify that the $classname is in fact a valid class or perform modularity
  * checks. Should only be used in rare cases to cast a known string that represents
  * some class to a classname. The preferred method to optain a classname should be using
  * the C::class syntax.
  */
  function classname_from_string_unsafe(string $classname)[]: classname<mixed>;
 
  /**
   * Creates a runtime KindOfClass (class pointer) from input $cn. Migration function
   * to eliminate implicit coercions from strings e.g. `$cn::func()`
   */
  function classname_to_class<T>(classname<T> $cn)[]: classname<T>;

  /**
   * Creates a runtime string from input class pointer $c. Migration function
   * to eliminate implicit coercions to strings e.g. `$mydict[$c]`
   */
  function class_to_classname<T>(classname<T> $c)[]: classname<T>;
} // namespace HH
