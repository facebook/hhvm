<?hh
function postinc_val(inout $x):mixed{ $r=$x; $x++; return $r; }
<<__DynamicallyCallable>>
function test($a) :mixed{
  try {
    return postinc_val(inout $a);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
    return null;
  }
}
<<__DynamicallyCallable>>
function foo() :mixed{
  return \HH\global_get('x');
}
<<__EntryPoint>>
function main_1104() :mixed{
var_dump(test(foo()));
}
