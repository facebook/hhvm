<?hh

abstract class C {
  abstract const ctx C as [write_props];
}

abstract class D {
  abstract const ctx C as [write_props] = [defaults];
}
