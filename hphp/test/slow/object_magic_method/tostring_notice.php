<?hh

class C {
  public function __toString()[] :mixed{
    echo "__toString called\n";
    return "string";
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();

  echo "==== explicit call ====\n";
  // explicitly calling __toString is the one thing that should _not_ notice
  var_dump($c->__toString());

  echo "==== explicit cast ====\n";
  var_dump((string) $c);

  echo "==== direct comparisons ====\n";
  var_dump(
    HH\Lib\Legacy_FIXME\lt("a", $c),
    HH\Lib\Legacy_FIXME\lte("a", $c),
    "a" == $c,
    "a" != $c,
    HH\Lib\Legacy_FIXME\gte("a", $c),
    HH\Lib\Legacy_FIXME\gt("a", $c),
  );
  var_dump(
    HH\Lib\Legacy_FIXME\lt($c, "z"),
    HH\Lib\Legacy_FIXME\lte($c, "z"),
    $c == "z",
    $c != "z",
    HH\Lib\Legacy_FIXME\gte($c, "z"),
    HH\Lib\Legacy_FIXME\gt($c, "z"),
  );

  echo "==== nested comparisons ====\n";
  $v = vec[$c];
  var_dump(
    HH\Lib\Legacy_FIXME\lt(vec["a"], $v),
    HH\Lib\Legacy_FIXME\lte(vec["a"], $v),
    vec["a"] == $v,
    vec["a"] != $v,
    HH\Lib\Legacy_FIXME\gte(vec["a"], $v),
    HH\Lib\Legacy_FIXME\gt(vec["a"], $v),
  );
  var_dump(
    HH\Lib\Legacy_FIXME\lt($v, vec["z"]),
    HH\Lib\Legacy_FIXME\lte($v, vec["z"]),
    $v == vec["z"],
    $v != vec["z"],
    HH\Lib\Legacy_FIXME\gte($v, vec["z"]),
    HH\Lib\Legacy_FIXME\gt($v, vec["z"]),
  );

  echo "==== concatenations ====\n";
  var_dump("a" . $c);
  var_dump($c . "s");
  $s = "a";
  $s .= $c;
  var_dump($s);
  $s = $c;
  $s .= "s";
  var_dump($s);

  echo "==== array-like write to string ====\n";
  $s = "apall";
  $s[0] = $c;
  var_dump($s);

  echo "==== array_diff ====\n";
  var_dump(array_diff(vec["int", "string"], vec[$c]));
  var_dump(array_diff(vec["int", $c], vec["string"]));

  echo "==== array_intersect ====\n";
  var_dump(array_intersect(vec["int", "string"], vec[$c]));
  var_dump(array_intersect(vec["int", $c], vec["string"]));
}
