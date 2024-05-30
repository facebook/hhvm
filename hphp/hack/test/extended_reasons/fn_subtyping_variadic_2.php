<?hh

class Super {}
class Sub extends Super {}

function foo_variadic_2((function(Sub, Super...):void) $_): void {}
function bar_variadic_2((function(Sub...):void) $x): void {
  foo_variadic_2($x);
}
