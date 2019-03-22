<?hh // partial


function globals_by_ref(): void {
  f(&$GLOBALS);
}
