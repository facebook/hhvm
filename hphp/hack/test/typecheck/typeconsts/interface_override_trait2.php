<?hh


trait T1 {
  abstract const type T = int;
}

interface I {
  abstract const type T = string;
}

abstract class B implements I { use T1; }
