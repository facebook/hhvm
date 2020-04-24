<?hh



<<__EntryPoint>> function foo(): void {
  $k = new stdclass;
  $k->paths = varray[new stdclass, new stdclass];
  foreach ($k->paths as $path) {
    echo $path->{0} . "\n";
  }
  echo "done\n";
}
