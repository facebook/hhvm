<?hh

function foo() :mixed{
  $bar = \HH\global_get('asd');
}
<<__EntryPoint>> function main(): void {
// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

foo();
var_dump(HH\global_key_exists('asd'));
}
