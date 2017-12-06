<?hh // strict

<<__Rx>>
function returnsReactive(): Rx<(function(): void)> {
  // UNSAFE
}

<<__Rx>>
function foo(): void {
  // returnsReactive is reactive
  $x = returnsReactive();
  // thing that returnsReactive returns is also reactive, no errors
  $x();
}
