<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public readonly ?P $prop=null, public ?P $mut_prop = null) { }
}

<<__EntryPoint>>
function main() : void {
  $p = new P();
  $p->prop = new P();
  $p->mut_prop = new P();
  $p->prop = readonly new P(); // ok
  $p->mut_prop = readonly $p->prop; // not ok
  $z = readonly $p->prop; // should be okay
  $w = $p->prop; // not okay
}
