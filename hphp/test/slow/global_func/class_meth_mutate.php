<?hh
class C {
  static public function func1() {
    return 1;
  }
}

<<__EntryPoint>>
function main() {
  $m = hh\class_meth(C::class, 'func1');
  var_dump(isset($m), empty($m));
  var_dump($m[0], $m[1]);
  var_dump($m is vec);
  var_dump($m());

  try {
    $m[1] = 'f';
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}
