<?hh

// I have these variants in the same case type so that we know one variant's
// where clause does not affect the checking of the other
case type CT<T> =
  | dict<T, mixed> where T as arraykey // should not error
  | keyset<T>; // should error

interface IFoo<T> {}

final class Foo<T> implements IFoo<T> {}
final class Bar<T> implements IFoo<T> {}

// I have these variants in the same case type so that we know one variant's
// where clause does not affect the checking of the other
case type CTWithAsBound<T> as IFoo<string> =
  | Foo<T> where T = string // should not error
  | Bar<T>; // should error

case type CT2<T1, T2> =
  // make sure one constraint in a where clause can be used to satisfy
  // requirements of types in another constraint in the same where clause
  | null where T1 as arraykey, T2 as dict<T1, mixed>;
