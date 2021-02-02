<?hh

abstract class Base {
  abstract const ctx C;
  abstract const ctx C = [write_props];
}

class Concrete {
  const ctx C = [];
  const ctx C = [rx];
}
