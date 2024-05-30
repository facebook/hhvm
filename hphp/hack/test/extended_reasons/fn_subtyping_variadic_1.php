<?hh

class Super {}
class Sub extends Super {}

function foo_variadic_1((function(Sub, Super):void) $_): void {}
function bar_variadic_1((function(Sub...):void) $x): void {
  foo_variadic_1($x);
}
