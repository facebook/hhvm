<?hh

/**
 * Tuple-like arrays have integer keys
 */

function test(): void {
  $a = array(true);
  take_int_indexed_array($a);
}

function take_int_indexed_array<Tv>(array<string, Tv> $a): void {}
