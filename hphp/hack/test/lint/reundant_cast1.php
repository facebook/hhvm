<?hh

function redundant_int_casts(int $i): void {
  (int) $i;
  (int) 42;
  (int) 10 + 100;
}

function redundant_string_casts(string $s): void {
  (string) $s;
  (string) 'hello';
  (string) 'hello'.'bye';
}

function redundant_float_casts(float $f): void {
  (float) $f + 42;   // Redundant due to $f
  (float) ($f + 42); // Redundant due to implicit coercion
}
