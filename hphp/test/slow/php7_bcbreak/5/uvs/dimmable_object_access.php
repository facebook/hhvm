<?hh
<<__EntryPoint>>
function entrypoint_dimmable_object_access(): void {


  error_reporting(-1);

  $foo = new stdClass();
  $foo->someprop = dict['baz' => 'quux'];

  $bar = 'someprop';

  try {
    var_dump($foo->$bar['baz']);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}
