<?hh // strict

<<__Rx>>
/* HH_FIXME[4110] */
function returnsReactive(): Rx<(function(): void)> {
}

<<__Rx>>
/* HH_FIXME[4110] */
function returnsNormal(): (function(): void) {
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
