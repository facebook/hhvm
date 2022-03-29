<?hh

enum class E : int {
  int list = 42;
}

<<__SupportDynamicType>>
class C {
  public function expectLabel(HH\EnumClass\Label<E, int> $x): void {
  }
}

function getC():~C {
  return new C();
}

<<__EntryPoint>>
function main(): void {
  $c = getC();
  $c->expectLabel(#list);
}
