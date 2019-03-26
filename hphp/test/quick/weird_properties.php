<?hh

function by_ref(&$ref) {}

function foo() {
  $k->paths = array(new stdclass, new stdclass);
  foreach ($k->paths as $path) {
    echo $path->{0} . "\n";
    by_ref(&$path->{24});
  }
  echo "done\n";
}

foo();
