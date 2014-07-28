<?hh

class A {
  const int X = 10;
  // static not allowed in constants
  const int Y = static::X;
}
