<?hh

abstract class A {
  abstract const type T1 as arraykey super int;
  abstract const type T2 super A as nonnull;
  abstract const type T2 super string as nonnull as mixed super nothing;
}
