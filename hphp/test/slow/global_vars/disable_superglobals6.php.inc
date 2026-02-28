<?hh

function foo() {
  bar(inout $_SERVER);
}

function bar(inout mixed $x) {}
