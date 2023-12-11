<?hh

function myfunc() :mixed{
  return 'quux';
}
<<__EntryPoint>>
function entrypoint_dimmable_object_call(): void {


  error_reporting(-1);

  $foo = new stdClass();
  $foo->someprop = dict['baz' => myfunc<>];

  $bar = 'someprop';

  var_dump($foo->$bar['baz']());
}
