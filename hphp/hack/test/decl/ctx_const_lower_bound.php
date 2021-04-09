<?hh

abstract class C {
  abstract const ctx C super [defaults];
}

abstract class D {
  abstract const ctx C super [defaults] = [];
}
