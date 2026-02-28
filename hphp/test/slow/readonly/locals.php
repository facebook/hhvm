<?hh

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() :mixed{
  $p = readonly new P(1);
  $p->i = 10;
}
