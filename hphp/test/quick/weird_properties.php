<?hh

function by_ref(&$ref) {}

<<__EntryPoint>> function foo(): void {
  $k->paths = array(new stdclass, new stdclass);
  foreach ($k->paths as $path) {
    echo $path->{0} . "\n";
    by_ref(&$path->{24});
  }
  echo "done\n";
}
