<?hh



<<__EntryPoint>> function foo(): void {
  $k->paths = varray[new stdclass, new stdclass];
  foreach ($k->paths as $path) {
    echo $path->{0} . "\n";
  }
  echo "done\n";
}
