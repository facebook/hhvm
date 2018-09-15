<?hh // strict
class X {}
function foo(array<?int, string> $x) : void {
  $x = new X();
}
