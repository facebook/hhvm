<?hh

function function_scope() {
  // Shouldn't try to capture $k
  $bar = () ==> {
    foreach (array(1,2,3,4) as $k) {
      var_dump($k);
    }
  };
  $bar();

  // Also shouldn't capture $x, $y:
  $baz = () ==> {
    $x = 12;
    $y = 13;
    echo $x . $y . "\n";
  };
  $baz();

  // But this will capture $z, however it will be undefined at closure
  // allocation time, emitting an undefined variable warning at the
  // allocation site (in non-repo mode).  It will also give an
  // ahead-of-type error from hh about $z not being defined.
  $quux = () ==> { var_dump(isset($z)); };
  $z = "function scope is weird\n";
  echo "z in parent is now: $z\n";
  $quux(); // print NULL
}

function_scope();
