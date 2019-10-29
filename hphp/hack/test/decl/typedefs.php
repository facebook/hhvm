<?hh

namespace MyNamespace {
    type MyString = string;
}

type MyString = string;
type MyInt = int;
type MyFloat = float;
type MyNum = num;
type MyBool = bool;

type MyNamespacedType = \MyNamespace\MyString;
