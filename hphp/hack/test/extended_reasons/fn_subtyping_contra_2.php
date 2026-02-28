<?hh

class Super {}
class Sub extends Super {}

function foo_2((function((function(Sub): void)): void) $_): void {}
function bar_2((function((function(Super): void)): void) $f): void {
  foo_2($f);
}
