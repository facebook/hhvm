<?hh

// We expect some typechecker errors in this one because we want the decl parser
// to be more accepting than the typechecker.

namespace MyNamespace {
  type MyString = string;
}

type MyString = string;
type MyInt = int;
type MyFloat = float;
type MyNum = num;
type MyBool = bool;

type MyNamespacedType = \MyNamespace\MyString;
