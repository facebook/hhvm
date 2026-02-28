<?hh

class Cls {
  const Array_CLASS_CONST = vec[42, 'foo'];
}

function f(inout int $i): void {}

function test(): void {
  f(inout Cls::Array_CLASS_CONST[0]);
}
