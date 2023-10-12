<?hh // strict

class A {
  const int X = 10;
  const int Z = self::X + 1;
}
