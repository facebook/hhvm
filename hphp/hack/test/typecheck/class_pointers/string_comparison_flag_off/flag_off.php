<?hh

class C1 {}

function compared_with_strings(string $s, vec<string> $v): void {
  if ($s === C1::class) {
  }
  if ($v === vec[C1::class]) {
  }
  switch ($s) {
    case C1::class:
      break;
    default:
      break;
  }
}
