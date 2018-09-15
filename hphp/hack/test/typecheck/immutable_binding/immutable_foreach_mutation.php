<?hh // experimental

function foo(Map<int, string> $map, string $prefix): string {
  $out = '';
  foreach ($map as k => v) {
    k .= ':';
    v .= ' ';
    $out .= k.v;
  }
  return $out;
}
