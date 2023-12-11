<?hh


<<__EntryPoint>>
function foo(): void {
  $k = new stdClass;
  $k->paths = vec[new stdClass, new stdClass];
  foreach ($k->paths as $path) {
    try {
      echo $path->{0}."\n";
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }
}
