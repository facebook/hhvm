<?hh

function myfunc() {
  return 'quux';
}
<<__EntryPoint>>
function entrypoint_dimmable_object_call(): void {


  error_reporting(-1);

  $foo = new stdclass();
  $foo->someprop = darray['baz' => 'myfunc'];

  $bar = 'someprop';

  var_dump($foo->$bar['baz']());
}
