<?hh //strict

class P {
  public function __construct(public int $i, public vec<int> $inner = vec[]) { }
}

<<__EntryPoint>>
function main() {
  $p = readonly new P(5);
  $p->inner[] = 5; // error, $p is readonly
}
