<?hh

abstract class C {
  abstract const ctx C;
}

class D extends C {
  const ctx C = [];
}
