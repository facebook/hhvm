<?hh

class P {
  public function __construct(public int $i, public ?P $inner = null) { }
}

<<__EntryPoint>>
function main() :mixed{
  $p = vec[vec[new P(5)]];
  $p[0][0] = readonly new P(5); // error, $p and $p[0] are mutable so you can't put a readonly thing in it
}
