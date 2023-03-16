<?hh

function redundant_cast(int $i, string $s, float $f, bool $b): void {
  (bool) $b;
  (float) $f;
  (string) $s;
  (int) $i;
}
