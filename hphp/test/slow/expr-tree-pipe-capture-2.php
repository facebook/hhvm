<?hh

<<file:__EnableUnstableFeatures('capture_pipe_variables')>>

class ET {
  static function makeTree(...$_args) {
    echo "ok.\n";
  }
}

<<__EntryPoint>>
function main() {
  42 |> function () {
    ET`$_ ==> $$`;
    ET`$$`;
    ET`() ==> { return $$; }`;
    ET`$_ ==> ($_ ==> $$)`;
    ET`() ==> { return ($_ ==> $$)($$); }`;
  } |> $$();

  42 |> () ==> {
    $x = () ==> 42 |> ET`() ==> { return ($_ ==> $$)($$); }`;
    $y = function () { return () ==> 42 |> ET`() ==> { return ($_ ==> $$)($$); }`; };
    return tuple($x, $y);
  } |> $$() |> tuple($$[0](), $$[1]()());
}

