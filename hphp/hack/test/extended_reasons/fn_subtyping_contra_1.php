<?hh

class Super {}
class Sub extends Super {}

function foo_1((function(Super): void) $_): void {}
function bar_1((function(Sub): void) $f): void {
  foo_1($f);
}
