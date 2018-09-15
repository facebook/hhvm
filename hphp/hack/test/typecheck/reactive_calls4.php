<?hh // strict

<<__Rx>>
function returnsReactive(): Rx<(function(): void)> {
  // UNSAFE
}

<<__Rx>>
function returnsNormal(): (function(): void) {
  // UNSAFE
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
