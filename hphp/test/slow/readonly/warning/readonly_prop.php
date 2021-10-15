<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public readonly ?P $prop=null) { }
}

<<__EntryPoint>>
function main() {
  $p = new P();
  $p->prop = new P();
  $z = readonly $p->prop; // should be okay
  $w = $p->prop; // not okay
}
