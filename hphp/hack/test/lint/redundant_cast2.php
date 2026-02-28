<?hh

// Not redundant
function redundant_cast2(mixed $m, arraykey $a, ?string $s): void {
  (string) $m;
  (float) $m;
  (bool) $m;
  (int) $m;

  (string) $a;
  (float) $a;
  (bool) $a;
  (int) $a;

  (string) $s;
  (float) $s;
  (bool) $s;
  (int) $s;
}
