<?hh

// We expect some typechecker errors in this one because we want the decl parser
// to be more accepting than the typechecker.

namespace MyNamespace {
  type MyString = string;
}


type MyArray1<T> = varray<T>;
type MyArray2<TK, TV> = darray<TK, TV>;
type MyDarray<TK, TV> = darray<TK, TV>;
type MyVarray<T> = varray<T>;
type MyVarrayOrDarray<TK, TV> = varray_or_darray<TK, TV>;
type MyString = string;
type MyInt = int;
type MyFloat = float;
type MyNum = num;
type MyBool = bool;
type MyMixed = mixed;
type MyNothing = nothing;
type MyNonnull = nonnull;
type MyDynamic = dynamic;
type MyArrayKey = arraykey;

type MyNamespacedType = \MyNamespace\MyString;

newtype MyNewtype as num = int;
newtype MyNewtypeWithoutConstraint = int;
