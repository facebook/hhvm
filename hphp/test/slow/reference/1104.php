<?hh
<<__DynamicallyCallable>>
function test($a) {
  try {
    return $a++;
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
    return null;
  }
}
<<__DynamicallyCallable>>
function foo() {
  return \HH\global_get('x');
}
<<__EntryPoint>>
function main_1104() {
var_dump(test(foo()));
}
