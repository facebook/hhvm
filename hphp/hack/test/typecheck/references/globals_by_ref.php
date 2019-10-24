<?hh // partial


function f(inout $_): void {}
function globals_by_ref(): void {
  f(inout $GLOBALS);
}
