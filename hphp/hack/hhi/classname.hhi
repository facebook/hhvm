<?hh // decl

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
