<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function lower_then_nullable_string<T super class<C> as ?string>(T $value): T {
  return $value;
}

function lower_then_nullable_arraykey<T super class<C> as ?arraykey>(
  T $value,
): T {
  return $value;
}

function lower_then_nullable_stringish<T super class<C> as ?Stringish>(
  T $value,
): T {
  return $value;
}

function lower_then_nullable_xhp_child<T super class<C> as ?XHPChild>(
  T $value,
): T {
  return $value;
}

function lower_then_union<T super class<C> as (string | float)>(
  T $value,
): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $nullable_string = lower_then_nullable_string(get_class_pointer());
  $nullable_string::method();
  $nullable_arraykey = lower_then_nullable_arraykey(get_class_pointer());
  $nullable_arraykey::method();
  $nullable_stringish = lower_then_nullable_stringish(get_class_pointer());
  $nullable_stringish::method();
  $nullable_xhp_child = lower_then_nullable_xhp_child(get_class_pointer());
  $nullable_xhp_child::method();
  $union = lower_then_union(get_class_pointer());
  $union::method();
}
