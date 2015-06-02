<?hh // strict

interface I {
  const type T = int;
}

abstract class P {
  const T = 0;
}

class C extends P implements I {}
