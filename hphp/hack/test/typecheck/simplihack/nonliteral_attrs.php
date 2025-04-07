<?hh
<<file: __EnableUnstableFeatures('simpli_hack')>>

function id<T>(T $x): T {
  return $x;
}

class FA1 implements HH\FunctionAttribute {
  public function __construct(public int $i)[] {}
}

class FA2 implements HH\FunctionAttribute {
  public function __construct(public string $s)[] {}
}

<<FA1(10), __SimpliHack(id(10), () ==> {}), FA2('hello')>>
function f(int $i): void {

}
