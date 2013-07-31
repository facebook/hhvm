<?php

function foo() {
  $k->paths = array(new stdclass, new stdclass);
  foreach ($k->paths as $path) {
    echo $path->{0} . "\n";
    $z =& $path->{24};
  }
  echo "done\n";
}

foo();
