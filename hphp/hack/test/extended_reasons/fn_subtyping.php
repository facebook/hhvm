<?hh

class Super {}
class Sub extends Super {}

function foo_0(Sub $_): void {}
function bar_0(Super $f): void {
  foo_0($f);
}

function foo_1((function(Super): void) $_): void {}
function bar_1((function(Sub): void) $f): void {
  foo_1($f);
}

function foo_2((function((function(Sub): void)): void) $_): void {}
function bar_2((function((function(Super): void)): void) $f): void {
  foo_2($f);
}

function foo_variadic_1((function(Sub, Super):void) $_): void {}
function bar_variadic_1((function(Sub...):void) $x): void {
  foo_variadic_1($x);
}


function foo_variadic_2((function(Sub, Super...):void) $_): void {}
function bar_variadic_2((function(Sub...):void) $x): void {
  foo_variadic_2($x);
}
