<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function group<Tk, Tv>(
  Traversable<Tv> $values,
  (function(Tv): ?Tk) $key_func,
): dict<Tk, vec<Tv>> {
  $result = dict[];
  foreach ($values as $value) {
    $key = $key_func($value);
    if ($key === null) {
      continue;
    }
    if (!\array_key_exists($key, $result)) {
      $result[$key] = vec[];
    }
    $result[$key][] = $value;
  }
  return $result;
}

function main() :mixed{
  for ($i = 0; $i < 4; $i++) {
    $d = group(dict['a' => 1, 'b' => 1, 'c' => 2, 'd' => 2], $v ==> $v + 1);
    var_dump($d);
  }
}

<<__EntryPoint>>
function main_hhbbc_group() :mixed{
main();
}
