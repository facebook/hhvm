<?hh
<<__DynamicallyCallable>>
function test($a) :mixed{
  try {
    return $a++;
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
