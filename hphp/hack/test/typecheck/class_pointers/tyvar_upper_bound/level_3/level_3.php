<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function lower_then_arraykey<T super class<C> as arraykey>(T $value): T {
  return $value;
}

function lower_then_stringish<T super class<C> as Stringish>(T $value): T {
  return $value;
}

function lower_then_xhp_child<T super class<C> as XHPChild>(T $value): T {
  return $value;
}

function lower_then_nullable_string<T super class<C> as ?string>(
  T $value,
): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $arraykey = lower_then_arraykey(get_class_pointer());
  $arraykey::method();
  $stringish = lower_then_stringish(get_class_pointer());
  $stringish::method();
  $xhp_child = lower_then_xhp_child(get_class_pointer());
  $xhp_child::method();
  $nullable_string = lower_then_nullable_string(get_class_pointer());
  $nullable_string::method();
}
