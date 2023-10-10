<?hh

class P {
  public function __construct(public readonly ?P $prop) { }
}

<<__EntryPoint>>
function main() :mixed{
  $x = readonly new P(null);
  $y = new P(null);
  $y->prop = $x; // ok
  echo "Done\n";
}
