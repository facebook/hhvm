<?hh

function f<reify T, Tu>(Tu $f): Tu {
  return $f;
}

function g(): void {
  f<int, string>(42);
}
