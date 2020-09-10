<?hh // strict

<<__Rx>>
function returnsReactive(): Rx<(function(): void)> {
  return <<__Rx>> () ==> {};
}

<<__Rx>>
function returnsNormal(): (function(): void) {
  return () ==> {};
}

<<__Rx>>
function takesReactive(Rx<(function(): void)> $x): void {}

<<__Rx>>
function takesNormal((function(): void) $x): void {}
<<__Rx>>
function foo(): void {
  // should be fine
  takesReactive(returnsReactive());
  // also okay
  takesNormal(returnsReactive());
  takesNormal(returnsNormal());
  // not okay
  takesReactive(returnsNormal());

}
