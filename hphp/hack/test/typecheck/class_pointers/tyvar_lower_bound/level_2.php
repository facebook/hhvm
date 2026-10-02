<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function id_string<T as string>(T $value): T {
  return $value;
}

function id_arraykey<T as arraykey>(T $value): T {
  return $value;
}

function id_stringish<T as Stringish>(T $value): T {
  return $value;
}

function lower_then_upper<T super class<C> as arraykey>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $string = id_string(get_class_pointer());
  $string::method();
  $arraykey = id_arraykey(get_class_pointer());
  $arraykey::method();
  $stringish = id_stringish(get_class_pointer());
  $stringish::method();
  $upper_arrives = lower_then_upper(get_class_pointer());
  $upper_arrives::method();
}
