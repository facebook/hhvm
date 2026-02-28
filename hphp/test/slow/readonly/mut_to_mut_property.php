<?hh

class P {
  public function __construct(public ?P $prop = null, public readonly ?P $ro_prop = null) { }
}

<<__EntryPoint>>
function main() :mixed{
  $x = readonly new P();
  $y = new P();
  $y->prop = new P(); // ok, mutable to mutable property
  $y->ro_prop = new P(); // ok, mutable to readonly property
  echo "Done\n";
}
