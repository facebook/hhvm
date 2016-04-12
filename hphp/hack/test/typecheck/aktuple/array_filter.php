<?hh

/**
 * We pretend that the only thing single-argument array_filter does is
 * remove nullability from it's value type (so vector-like arrays remain
 * vector like).
 */
function test(): array<int> {
  return array_filter(array(1, 2, 3));
}
