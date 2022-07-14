<?hh // strict
<<file: __EnableUnstableFeatures('modules')>>

module foo;

newtype N = int;

internal newtype NI = int;

trait T {
}

internal trait TI {
}

interface I {
}

internal interface II {
}

enum E: int {
  Red = 3;
}

internal enum EI: int {
  Red = 3;
}
