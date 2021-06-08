<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : mixed {
  int A = 42;
}

class C {
  static public function g<T>(HH\EnumClass\Label<E, T> $label) : T {
    return E::valueOf($label);
  }
}

<<__EntryPoint>>
function main(): void {
  C::g#AUTO332
}
