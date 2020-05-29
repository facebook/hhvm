<?hh // strict
<<__Rx>>

function foo() : Rx<(function (int) : void)> {
  throw new Exception();
}
