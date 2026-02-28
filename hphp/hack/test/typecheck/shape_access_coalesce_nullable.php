<?hh

function f(?shape('a' => int) $s): void {
  $s['a'] ?? 42;  // Fine
  $s['b'] ?? 42; // Not fine
  $s as nonnull;
  $s['a'] ?? 42;  // Fine
  $s['b'] ?? 42; // Not fine
}
