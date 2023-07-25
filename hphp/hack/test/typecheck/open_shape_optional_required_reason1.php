<?hh

function foo(shape('field' => string, ...) $_): void {}

function bar(shape(...) $s): void {
  foo($s);
}
