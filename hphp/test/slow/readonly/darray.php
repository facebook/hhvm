<?hh //strict

<<file: __EnableUnstableFeatures('readonly')>>

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() {
  $ro = readonly new P(1);
  $mut = new P(1);
  $p = darray[1 => $ro, 2 => $mut];
  $mut = $p; // error, assigning readonly value to mutable
}
