<?hh

<<__NEVER_INLINE>>
function f(dict<string, int> $d) {
  try {
    return $d['a'];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main() {
  $d = \__hhvm_intrinsics\launder_value(dict[]);
  f($d);
  f($d);
}
