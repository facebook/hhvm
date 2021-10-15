<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public ?P $prop) { }
}

<<__EntryPoint>>
function main() {
  $x = readonly new P(null);
  $y = new P(null);
  $y->prop = $x; // error, $x is readonly but $y is not readonly property
}
