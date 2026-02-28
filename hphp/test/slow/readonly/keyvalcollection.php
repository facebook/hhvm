<?hh

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() :mixed{
  $ro = readonly new P(1);
  $mut = new P(1);
  $p = Map {1 => $ro, 2 => $mut};
  $p[2]->i = 4; // error, $p[2] is readonly because $p is readonly
}
