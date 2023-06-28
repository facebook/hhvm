<?hh

<<__ALWAYS_INLINE>>
function lookupPair($id): ?(int, int) {
  switch ($id) {
    case 0:
      return tuple(0, 0);
    case 1:
      return tuple(1, 0);
    case 3:
      return tuple(2, 0);
    default:
      return null;
  }
}

<<__NEVER_INLINE>>
function run($id) :mixed{
  $d = dict[0 => 0, 1 => 1, 2 => 2];
  list($curr, $_) = lookupPair($id);

  return $d[$curr];
}

<<__EntryPoint>>
function main() :mixed{
  for ($i = 0; $i < 3; $i++) {
    try {
      run(__hhvm_intrinsics\launder_value($i));
    } catch (Exception $e) {}
  }
  var_dump("success");
}
