<?hh

final class MyInt {}
final class MyString {}
final class MyBool {}

case type LiftableTo<+T> =
  | int where T super MyInt
  | string where T super MyString
  | bool where T super MyBool;

function test1<T>((LiftableTo<T>, LiftableTo<T>) $tuple): T {
  if ($tuple is (int, int)) {
    return new MyInt();
  } else if ($tuple is (string, string)) {
    return new MyString();
  } else {
    return
      new MyBool(); // this should error, e.g. for T = (MyInt | MyString), $tuple: (int, string)
  }
}

function test_either<T>((LiftableTo<T>, LiftableTo<T>) $tuple, bool $test): T {
  if ($tuple is (int, bool)) {
    // we should infer that MyInt < T && MyBool < T
    return $test ? new MyBool() : new MyInt();
  } else {
    throw new Exception();
  }
}

function test2<T>((LiftableTo<T>, LiftableTo<T>) $tuple): T {
  if ($tuple is (arraykey, arraykey)) {
    return
      new MyInt(); // this should error, e.g. for T = MyString, $tuple: (string, string)
  } else {
    // We assume: (MyBool <: T || ((MyBool <: T) && ((MyString <: T || MyInt <: T))) || (((MyString <: T || MyInt <: T)) && (MyBool <: T)))
    // but that doesn't get simplified to MyBool <: T && (...)
    return new MyBool(); // errors due to lack of prop simplification
  }
}

function test3<T>((LiftableTo<T>, LiftableTo<T>) $tuple): T {
  if (
    $tuple is (arraykey, arraykey) ||
    $tuple is (arraykey, bool) ||
    $tuple is (bool, arraykey)
  ) {
    throw new Exception();
  } else {
    return
      new MyBool(); // errors due to (A, B) | (C, D) -> (A|C, B|D) approximation
  }
}
