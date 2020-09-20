<?hh

class C {
  public function __toString() {
    echo "__toString called\n";
    return "string";
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();

  echo "==== explicit call ====\n";
  // explicitly calling __toString is the one thing that should _not_ notice
  var_dump($c->__toString());

  echo "==== explicit cast ====\n";
  var_dump((string) $c);

  echo "==== direct comparisons ====\n";
  var_dump(
    "a" <  $c,
    "a" <= $c,
    "a" == $c,
    "a" != $c,
    "a" >= $c,
    "a" >  $c,
  );
  var_dump(
    $c <  "z",
    $c <= "z",
    $c == "z",
    $c != "z",
    $c >= "z",
    $c >  "z",
  );

  echo "==== nested comparisons ====\n";
  $v = vec[$c];
  var_dump(
    vec["a"] <  $v,
    vec["a"] <= $v,
    vec["a"] == $v,
    vec["a"] != $v,
    vec["a"] >= $v,
    vec["a"] >  $v,
  );
  var_dump(
    $v <  vec["z"],
    $v <= vec["z"],
    $v == vec["z"],
    $v != vec["z"],
    $v >= vec["z"],
    $v >  vec["z"],
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
  var_dump(array_diff(varray["int", "string"], varray[$c]));
  var_dump(array_diff(varray["int", $c], varray["string"]));

  echo "==== array_intersect ====\n";
  var_dump(array_intersect(varray["int", "string"], varray[$c]));
  var_dump(array_intersect(varray["int", $c], varray["string"]));
}
