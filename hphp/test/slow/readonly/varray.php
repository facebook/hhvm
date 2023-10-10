<?hh

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() :mixed{
  $ro = readonly new P(1);
  $mut = new P(1);
  $p = varray[$ro, $mut];
  $p[1]->i = 4; // error, $p[1] is readonly because $p is readonly
}
