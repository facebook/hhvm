<?hh

type MyAlias = shape('foo'=>E);

<<__EntryPoint>>
function entrypoint_ReflectionTypeAlias5(): void {

  try {
    $x = new ReflectionTypeAlias('MyAlias');
    var_dump($x->getResolvedTypeStructure());
  } catch (ReflectionException $ex) {
    echo 'ReflectionException: ', $ex->getMessage(), "\n";
  }
}
