<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function id_string_union<T as (string | float)>(T $value): T {
  return $value;
}

function id_arraykey_union<T as (arraykey | float)>(T $value): T {
  return $value;
}

function id_stringish_union<T as (Stringish | vec<int>)>(T $value): T {
  return $value;
}

function id_xhp_child_union<T as (XHPChild | bool)>(T $value): T {
  return $value;
}

function lower_then_upper<T super class<C> as (string | float)>(
  T $value,
): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $string_union = id_string_union(get_class_pointer());
  $string_union::method();
  $arraykey_union = id_arraykey_union(get_class_pointer());
  $arraykey_union::method();
  $stringish_union = id_stringish_union(get_class_pointer());
  $stringish_union::method();
  $xhp_child_union = id_xhp_child_union(get_class_pointer());
  $xhp_child_union::method();
  $upper_arrives = lower_then_upper(get_class_pointer());
  $upper_arrives::method();
}
