<?hh // partial

function takes_ref(int &$v): void {}

function test(): void {
  $x = array(42);
  takes_ref(&$x[0]);
}
