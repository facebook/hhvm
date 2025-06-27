<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Base<T> {
  public static function foo<Tu>(Tu $arg): T where T super vec<Tu> {
    return vec[$arg];
  }
}

final class Derived<Ta> extends Base<vec<Ta>> {}

function ref_it(): void {
  $fptr = Derived::foo<>;
  hh_expect_equivalent<
    HH\FunctionRef<(readonly function<Ta super Tu, Tu as Ta>(Tu): vec<Ta>)>,
  >($fptr);
}
