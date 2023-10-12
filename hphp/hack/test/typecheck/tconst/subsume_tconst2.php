<?hh // strict

interface I {
  abstract const type T as arraykey;
}

class C implements I {
  const type T = mixed;
}
