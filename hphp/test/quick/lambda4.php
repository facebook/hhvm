<?hh

<<__EntryPoint>> function function_scope(): void {
  // Shouldn't try to capture $k
  $bar = () ==> {
    foreach (vec[1,2,3,4] as $k) {
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

  // But this will capture $z, however it will be uninit at closure
  // allocation time (in non-repo mode). It will give a warning
  // closure call time. It will also give an ahead-of-type error
  // from hh about $z not being defined.
  $quux = () ==> { var_dump(isset($z)); };
  $z = "function scope is weird\n";
  echo "z in parent is now: $z\n";
  $quux(); // print NULL
}
