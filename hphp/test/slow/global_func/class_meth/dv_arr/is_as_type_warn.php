<?hh
class A {
  static public function func1() {
    return 1;
  }
}

function test_is_type($x) {
  var_dump(is_array($x));
  var_dump(HH\is_varray($x));
  var_dump(HH\is_vec($x));
  var_dump($x is vec);

  var_dump(HH\is_darray($x));
  var_dump(HH\is_dict($x));
  var_dump($x is dict);

  var_dump(HH\is_any_array($x));
  var_dump(HH\is_list_like($x));
  var_dump($x is HH\Container);
  var_dump($x is HH\Traversable);

  var_dump(HH\is_class_meth($x));
}

function test_as_type($x) {
  try {
    var_dump($x as vec);
  } catch (TypeAssertionException $e) {
    print "Caught: ".$e->getMessage()."\n";
  }
  try {
    var_dump($x as int);
  } catch (TypeAssertionException $e) {
    print "Caught: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() {
  print "* test_is_type - varray:\n";
  test_is_type(varray[1, 2, 3]);

  print "* test_is_type - clsmeth:\n";
  test_is_type(HH\class_meth(A::class, 'func1'));

  print "* test_as_type:\n";
  test_as_type(HH\class_meth(A::class, 'func1'));
}
