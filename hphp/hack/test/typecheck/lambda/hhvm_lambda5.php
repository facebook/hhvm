<?hh

function foo() {
  $y = 12;
  $bar = () ==> {
    $baz = () ==> {
      $quux = () ==> {
        return $y; // capture of $y better propagate out
      };

      $foo = 12;

      return $quux;
    };

    return $baz;
  };

  return $bar;
}

function main() {
  $k = foo();
  var_dump($k instanceof Closure); // true
  $y = $k();
  var_dump($y instanceof Closure); // true
  $z = $y();
  var_dump($z instanceof Closure); // true
  var_dump($z()); // 12
}

main();
