<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

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
function f(int $i): mixed {

}

function reflect () :mixed{
  $rf = new ReflectionFunction("f");
  var_dump($rf->getAttributeClass(FA1::class)->i); // 10
  var_dump($rf->getAttributeClass(FA2::class)->s); // 'hello'
  var_dump(is_null($rf->getAttributeClass(__SimpliHack::class))); // true
}

<<__EntryPoint>>
function main_simplihack() :mixed{
  reflect();
}
