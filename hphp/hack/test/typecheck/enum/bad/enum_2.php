<?hh // strict

abstract class Enum {
  abstract const type TInner;
}

class Foo extends Enum {
  const type TInner = mixed;
  const int FOO = 0;
  const string BAR = "lol";
}

class Bar extends Enum {
  const type TInner = arraykey;
  const int FOO = 0;
  const string BAR = "lol";
}
