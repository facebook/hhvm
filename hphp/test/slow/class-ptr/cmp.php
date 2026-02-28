<?hh

<<__DynamicallyReferenced>> class foo {}

function getStr() :mixed{
  return __hhvm_intrinsics\launder_value("foo");
}

function getLazyClass() :mixed{
  return __hhvm_intrinsics\launder_value(foo::class);
}
function getClass() :mixed{
  return __hhvm_intrinsics\launder_value(HH\classname_to_class("foo"));
}

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) :mixed{
  wrap(() ==> { echo var_export_pure($x)." === ".var_export_pure($y)." = "; var_dump($x === $y); });
  wrap(() ==> { echo var_export_pure($x)."  eq ".var_export_pure($y)." = "; var_dump(HH\Lib\Legacy_FIXME\eq($x, $y)); });
  wrap(() ==> { echo var_export_pure($x)."  lt ".var_export_pure($y)." = "; var_dump(HH\Lib\Legacy_FIXME\lt($x, $y)); });
  wrap(() ==> { echo var_export_pure($x)." lte ".var_export_pure($y)." = "; var_dump(HH\Lib\Legacy_FIXME\lte($x, $y)); });
  wrap(() ==> { echo var_export_pure($x)."  gt ".var_export_pure($y)." = "; var_dump(HH\Lib\Legacy_FIXME\gt($x, $y)); });
  wrap(() ==> { echo var_export_pure($x)." gte ".var_export_pure($y)." = "; var_dump(HH\Lib\Legacy_FIXME\gte($x, $y)); });
  print("\n");
}




// Spaced so the test number matches the line number - 40
function getTestcases() : vec<mixed>{
  return vec[
    true,
    false,
    0,
    1,
    0.0,
    1.0,
    "foo",
    foo::class,
    HH\classname_to_class("foo"),
    darray(vec['foo']),
    vec['foo'],
    vec['foo'],
    dict[0 => 'foo'],
    dict[0 => 'foo'],
    keyset['foo'],
  ];
}

function getTestcase($test_cases, $num) :mixed{
  return __hhvm_intrinsics\launder_value($test_cases[$num]);
}

function comp_test($x) :mixed{
  $test_cases = getTestcases();
  for ($i = 0; $i < count($test_cases); $i++) {
    print("Test ".$i."\n");
    comp($x, getTestcase($test_cases, $i));
    comp(getTestcase($test_cases, $i), $x);
  }
}

<<__EntryPoint>>
function main(): void {
  comp_test(getStr());
  print("--- test lazy class --- \n");
  comp_test(getLazyClass());
  print("--- test class --- \n");
  comp_test(getClass());

}
