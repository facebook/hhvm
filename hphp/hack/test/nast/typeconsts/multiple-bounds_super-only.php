<?hh

abstract class A {
  abstract const type T0;
  abstract const type T1 super string;
  abstract const type T2 super arraykey super int;
  abstract const type T3 super int super arraykey = arraykey;
}
