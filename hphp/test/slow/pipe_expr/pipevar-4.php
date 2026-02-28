<?hh

class Wrapper {
  public function __construct(public darray $val) { var_dump("Make wrapper"); }
}

function beep($x) :mixed{
  if ($x is Wrapper) {
    var_dump("beep: <Wrapper>");
  } else if (is_array($x)) {
    var_dump("beep: <array>");
  } else {
    var_dump("beep: ".$x);
  }
  return $x;
}
function wrap($x) :mixed{
  return new Wrapper($x);
}
function unwrap($y) :mixed{
  return $y->val;
}

function main($bar) :mixed{
  $foo = "Hello!";
  $out = vec[1, 2, 3]
    |> array_map($x ==> $x + beep(1), $$)
    |> array_merge(
      vec[50, 60, 70]
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
function main_pipevar_4() :mixed{
main("Goodbye");
}
