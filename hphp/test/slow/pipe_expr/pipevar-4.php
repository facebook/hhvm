<?hh // decl

class Wrapper {
  public function __construct(public array $val) { var_dump("Make wrapper"); }
}

function beep($x) {
  if ($x is Wrapper) {
    var_dump("beep: <Wrapper>");
  } else if (is_array($x)) {
    var_dump("beep: <array>");
  } else {
    var_dump("beep: ".$x);
  }
  return $x;
}
function wrap($x) {
  return new Wrapper($x);
}
function unwrap($y) {
  return $y->val;
}

function main($bar) {
  $foo = "Hello!";
  $out = varray[1, 2, 3]
    |> array_map($x ==> $x + beep(1), $$)
    |> array_merge(
      varray[50, 60, 70]
        |> array_map($x ==> $x * beep(2), $$)
        |> array_filter($$, $x ==> $x != beep(100)),
      $$)
    |> array_filter($$, $x ==> $x != beep(3))
    |> wrap($$)
    |> beep($$)
    |> unwrap($$)
    |> beep($$)
    |> array_map($x ==> "STR: $x", $$);

  var_dump($foo);
  var_dump($out);
  var_dump($bar);
}


<<__EntryPoint>>
function main_pipevar_4() {
main("Goodbye");
}
