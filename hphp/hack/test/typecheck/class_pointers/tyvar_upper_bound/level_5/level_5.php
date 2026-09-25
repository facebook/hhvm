<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function lower_then_string_union<T super class<C> as (string | float)>(
  T $value,
): T {
  return $value;
}

function lower_then_arraykey_union<T super class<C> as (arraykey | float)>(
  T $value,
): T {
  return $value;
}

function lower_then_stringish_union<
  T super class<C> as (Stringish | vec<int>),
>(T $value): T {
  return $value;
}

function lower_then_xhp_child_union<T super class<C> as (XHPChild | bool)>(
  T $value,
): T {
  return $value;
}

function lower_then_classname<T super class<C> as classname<C>>(
  T $value,
): T {
  return $value;
}

function lower_then_class_or_classname<
  T super class<C> as class_or_classname<C>,
>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $string_union = lower_then_string_union(get_class_pointer());
  $string_union::method();
  $arraykey_union = lower_then_arraykey_union(get_class_pointer());
  $arraykey_union::method();
  $stringish_union = lower_then_stringish_union(get_class_pointer());
  $stringish_union::method();
  $xhp_child_union = lower_then_xhp_child_union(get_class_pointer());
  $xhp_child_union::method();
  $classname = lower_then_classname(get_class_pointer());
  $classname::method();
  $class_or_classname = lower_then_class_or_classname(get_class_pointer());
  $class_or_classname::method();
}
