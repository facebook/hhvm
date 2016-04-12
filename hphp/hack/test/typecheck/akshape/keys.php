<?hh

/**
 * Shape-like arrays remember key types
 */

function test(): void {
  $a = array();
  $a['aaa'] = true;
  take_int_indexed_array($a);
}

function take_int_indexed_array<Tv>(array<int, Tv> $a): void {}
