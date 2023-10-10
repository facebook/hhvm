<?hh

class P {
  public function __construct(public int $i, public ?P $inner = null) { }
}

<<__EntryPoint>>
function main() :mixed{
  $p = readonly vec[new P(5)];
  $p[0] = new P(5); // ok
  echo "Done\n";
}
