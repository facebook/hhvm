<?hh

abstract class C {
  abstract const ctx C as [write_props];
}

class D {
  const ctx C as [write_props] = [defaults];
}
