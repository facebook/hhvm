<?hh

final class MyNull {}
final class MyInt {}
final class MyString {}

case type MyCaseType<T> =
  | null where T as MyNull
  | int where T as MyInt
  | string where T as MyString;

function foo<T>(MyCaseType<T> $x, T $t): MyString {
  if ($x is ?int) {
    // T as MyNull ||| T as MyInt (ignore disjunction)
    throw new Exception();
  } else {
    // T as MyString
    return $t;
  }
}
