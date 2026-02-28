<?hh

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() :mixed{
  $ro = readonly new P(1);
  $mut = new P(1);
  $p = shape("a" => $ro, "b" => $mut);
  $p["a"]->i = 4; // error, $p is mutable
}
