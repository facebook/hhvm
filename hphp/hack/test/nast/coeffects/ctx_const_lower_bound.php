<?hh

abstract class C {
  abstract const ctx C super [defaults];
}

class D {
  const ctx C super [defaults] = [];
}
