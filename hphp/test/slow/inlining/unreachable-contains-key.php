<?hh

abstract final class GetValueStatics {
  public static $x = 0;
}

<<__NEVER_INLINE>>
function get_value() {
  GetValueStatics::$x++;
  if ((GetValueStatics::$x % 2) == 0) return 1;
  return new stdclass;
}

function contains_key(KeyedContainer $c, $k) {
  return \array_key_exists($k, $c);
}

function main($base) {
  try {
    $v = get_value();
    return contains_key($base, $v);
  } catch (Exception $e) {
    printf("%s\n", $e->getMessage());
  }
}


<<__EntryPoint>>
function main_unreachable_contains_key() {
for ($i = 0; $i < 100; ++$i) {
  main(darray[]);
  main(keyset[]);
}
}
