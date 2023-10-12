<?hh // strict

enum E: string as string {
  V1 = 'V1';
  V2 = 'V2';
}

function f(E $x): nonnull {
  return $x;
}
