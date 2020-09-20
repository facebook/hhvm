<?hh // strict

class C {
  const type T = arraykey;
}

class D extends C {
  const type T as mixed = int;
}
