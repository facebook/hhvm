<?hh // decl

/**
 * For any class C, C::class creates a classname<C>. Due to the properties
 * of opaque types, C::class is the only way of obtaining a classname.
 */
newtype classname<+T> = string;
