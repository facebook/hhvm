<?hh

/**
 * Tuple-like arrays have integer keys
 */

function test(): void {
  $a = vec[true];
  take_int_indexed_array($a);
}

function take_int_indexed_array<Tv>(darray<string, Tv> $a): void {}
