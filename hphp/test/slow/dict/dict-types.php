<?hh

function take_dict1(dict $foo): dict {
  $foo["1"] = "dict1";
  return $foo;
}

function take_dict2(dict<Foo> $bar): dict<Foo> {
  $bar[2] = "dict2";
  return $bar;
}

function take_dict3(dict<Foo,> $baz): dict<Foo,> {
  $baz[1] = "dict3";
  return $baz;
}

function take_dict4(dict<Foo, Bar> $biz): dict<Foo, Bar> {
  $biz["2"] = "dict4";
  return $biz;
}

function take_array(array $arr): array {
  $arr["3"] = "arr-str";
  $arr[3] = "arr-num";
  return $arr;
}

function main() {
  $d = dict["x" => "y"];
  $a = array("a" => "b");

  take_dict1($d)
    |> take_dict2($$)
    |> take_dict3($$)
    |> take_dict4($$)
    |> take_array($$)
    |> var_dump($$);

  take_dict1($a)
    |> take_dict2($$)
    |> take_dict3($$)
    |> take_dict4($$)
    |> take_array($$)
    |> var_dump($$);
}

main();
