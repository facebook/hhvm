<?hh


<<__EntryPoint>>
function foo(): void {
  $k = new stdclass;
  $k->paths = varray[new stdclass, new stdclass];
  foreach ($k->paths as $path) {
    try {
      echo $path->{0}."\n";
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }
}
