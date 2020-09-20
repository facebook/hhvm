<?hh
<<__EntryPoint>>
function entrypoint_dimmable_object_access(): void {


  error_reporting(-1);

  $foo = new stdclass();
  $foo->someprop = darray['baz' => 'quux'];

  $bar = 'someprop';

  var_dump($foo->$bar['baz']);
}
