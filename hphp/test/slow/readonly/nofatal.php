<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

// Does not error if opts do not enable enforcement
class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() {
  $p = readonly new P(1);
  $p->i = 10;
}
