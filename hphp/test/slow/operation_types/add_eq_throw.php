<?hh
<<__EntryPoint>> function main(): void {
  $a = 0;
  $e = keyset[];
  try {
    $a += HH\Lib\Legacy_FIXME\cast_for_arithmetic($e);
  } catch (Exception $e) {
    var_dump($a);
  }
}
