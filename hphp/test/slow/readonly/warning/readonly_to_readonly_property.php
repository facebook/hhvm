<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public readonly ?P $prop) { }
}

<<__EntryPoint>>
function main() {
  $x = readonly new P(null);
  $y = new P(null);
  $y->prop = $x; // ok
}
