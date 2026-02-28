<?hh

class ET {
  static function makeTree(...$_args) {
    echo "ok.\n";
  }
}

<<__EntryPoint>>
function main() {
  42 |> ET`$_ ==> $$`;
  42 |> ET`$$`;
  42 |> ET`() ==> { return $$; }`;
  42 |> ET`$_ ==> ($_ ==> $$)`;
  42 |> ET`() ==> { return ($_ ==> $$)($$); }`;
  $x = () ==> 42 |> ET`() ==> { return ($_ ==> $$)($$); }`;
  $x();
  $y = function () { return () ==> 42 |> ET`() ==> { return ($_ ==> $$)($$); }`; };
  $y()();
}
