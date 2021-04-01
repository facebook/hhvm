<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() {
  $p = readonly new P(1);
  $p->i = 10;
}
