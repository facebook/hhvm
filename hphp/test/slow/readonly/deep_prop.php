<?hh //strict

class P {
  public function __construct(public int $i, public ?P $inner = null) { }
}

<<__EntryPoint>>
function main() {
  $p = readonly new P(1, new P(2));
  $p->inner->i = 10;
}
