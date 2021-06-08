<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : mixed {
  int A = 42;
}

class C {
  public function f<T>(HH\EnumClass\Label<E, T> $label) : T {
    return E::valueOf($label);
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new C();

  $c->f<int>#AUTO332
}
