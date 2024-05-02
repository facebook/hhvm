<?hh

class Super {}
class Sub extends Super {}

function foo(Sub $_): void {}

function bar_1((string,Super,bool) $xyz): void {
  list($_, $y, $_) = $xyz;
  foo($y);
}
