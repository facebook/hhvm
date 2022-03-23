<?hh

function takes_param(inout int $x): void {}

function call_it(): void {
  $x = 1;
  /* HH_FIXME[4182] quickfix should still work on next line */
  takes_param($x);
}
