<?hh // strict

interface I {
  const T = 0;
}

abstract class P {
  const type T = int;
}

class C extends P implements I {}
