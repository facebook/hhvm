<?hh // strict

abstract class C {
  abstract const type T as mixed = arraykey;
}

class D extends C {
  const type T as mixed = int;
}
