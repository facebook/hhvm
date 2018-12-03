<?hh // strict

function f<reify T, Tu>(Tu $f): Tu {
  return $f;
}

function g(): void {
  f<reify int, string>(42);
}
