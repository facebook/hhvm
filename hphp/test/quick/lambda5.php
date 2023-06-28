<?hh

function foo() :mixed{
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

<<__EntryPoint>> function main(): void {
  $k = foo();
  var_dump($k is Closure); // true
  $y = $k();
  var_dump($y is Closure); // true
  $z = $y();
  var_dump($z is Closure); // true
  var_dump($z()); // 12
}
