<?hh

class Super {}
class Sub extends Super {}

function foo(Sub $_): void {}

function bar_1(Super $f): void {
  $g = $f;
  foo($g);
}

function bar_2(Super $f): void {
  $g = $f;
  $h = $g;
  foo($h);
}

function bar_3(Super $f): void {
  $g = $f;
  $h = $g;
  $i = $h;
  foo($i);
}
