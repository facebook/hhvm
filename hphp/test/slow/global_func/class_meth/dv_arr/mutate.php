<?hh
class C {
  static public function func1() {
    return 1;
  }
}

<<__EntryPoint>>
function main() {
  $m = hh\class_meth(C::class, 'func1');
  var_dump(empty($m));
  var_dump(isset($m), isset($m[1]), isset($m[2]), isset($m['s']));
  var_dump($m[0], $m[1]);
  var_dump(HH\is_varray($m));
  var_dump($m());

  try {
    $m[1] = 'f';
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}
