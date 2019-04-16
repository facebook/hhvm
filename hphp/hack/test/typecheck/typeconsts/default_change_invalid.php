<?hh

abstract class AA {
  abstract const type T as arraykey = int;
}

class A extends AA {
  const type T = bool;
}
