<?hh // strict

<<__Rx>>
/* HH_FIXME[4336] */
function returnsReactive(): Rx<(function(): void)> {
}

<<__Rx>>
function foo(): void {
  // returnsReactive is reactive
  $x = returnsReactive();
  // thing that returnsReactive returns is also reactive, no errors
  $x();
}
