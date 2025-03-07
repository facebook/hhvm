<?hh

<<file: __EnableUnstableFeatures('class_type')>>

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
   * For any class C, nameof C creates a classname<C> string. The typechecker
   * enforces that C is a defined class, but it does not enforce module or
   * package boundaries. */
  <<__NoAutoDynamic>>
  newtype classname<+T> as typename<T> = typename<T>;

  <<__NoAutoDynamic>>
  newtype concreteclassname<+T> as classname<T> = classname<T>;

  /**
   * Similar to vec_or_dict, this type is a migration type to cover places that
   * need to handle both class<T> and classname<T> without raising notices and it
   * should generally never be used. When typechecker flag class_sub_classname=true,
   * this type is equivalent to classname<T>.
   */
  <<__NoAutoDynamic>>
  type class_or_classname<T> = classname<T>;

  /**
   * Creates a runtime KindOfClass (class pointer) from input $cn. Migration function
   * to eliminate implicit coercions from strings e.g. `$cn::func()`
   */
  function classname_to_class<T>(readonly class_or_classname<T> $cn)[]: class<T>;

  /**
   * Creates a runtime string from input class pointer $c. Migration function
   * to eliminate implicit coercions to strings e.g. `$mydict[$c]`
   */
  function class_to_classname<T>(readonly class_or_classname<T> $c)[]: classname<T>;
} // namespace HH
