<?hh

class Super {}
class Sub extends Super {}


function foo(Sub $_): void {}

function bar(Super $f): void {
  $g = $f;
  foo($g);
}
