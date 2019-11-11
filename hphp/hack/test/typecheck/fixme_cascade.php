//// a.php
<?hh

function foo(
  /* HH_FIXME[4110] */
  int $i
): void {}

//// b.php
<?hh

function test(): void {
  foo('h');
}
