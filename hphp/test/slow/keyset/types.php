<?hh

function take_keyset1(keyset $foo): keyset {
  $foo[] = "keyset1";
  return $foo;
}

function take_keyset2(keyset<Foo> $bar): keyset<Foo> {
  $bar[] = "keyset2";
  return $bar;
}

function take_keyset3(keyset<Foo,> $baz): keyset<Foo,> {
  $baz[] = "keyset3";
  return $baz;
}

function take_keyset4(keyset<Foo, Bar> $biz): keyset<Foo, Bar> {
  $biz[] = "keyset4";
  return $biz;
}

function take_array(array $arr): array {
  $arr[] = "arr-str";
  $arr[] = 10;
  return $arr;
}

function main() {
  $d = keyset["x", "y"];
  $a = array("a" => "b");

/*
  take_keyset1($d)
    |> take_keyset2($$)
    |> take_keyset3($$)
    |> take_keyset4($$)
    |> take_array($$)
    |> var_dump($$);

  take_keyset1($a)
    |> take_keyset2($$)
    |> take_keyset3($$)
    |> take_keyset4($$)
    |> take_array($$)
    |> var_dump($$);
*/
}

set_error_handler(
  (int $errno,
   string $errstr,
   string $errfile,
   int $errline,
   array $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    return true;
  }
);

main();
