<?hh

function bad(Vector<int> $vi, Vector<arraykey> $va): void {
  if ($vi == $va) {
    $vi[0] = 'str';
  }
}

function typed(Vector<int> $vi, Vector<arraykey> $va): int {
  bad($vi, $va);
  return $vi[0];
}

<<__EntryPoint>>
function main(): void {
  $vi = Vector { 1 };
  $va = Vector { 1 };
  typed($vi, $va);
}
