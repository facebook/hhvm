<?hh

function append(
  dict<int, vec<int>> $store,
  int $key,
  int $val,
): dict<int, vec<int>> {
  // Side-effect.
  \usleep(1);

  if (\array_key_exists($key, $store)) {
    $store[$key][] = $val;
  } else {
    $store[$key] = vec[$val];
  }
  return $store;
}

<<__EntryPoint>>
function main(): void {
  $store = abs(rand()) >= 0 ? append(dict[], 42, 47) : dict[];
  var_dump($store[42]);
}
