<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function lower_then_string<T super class<C> as string>(T $value): T {
  return $value;
}

function lower_then_arraykey<T super class<C> as arraykey>(T $value): T {
  return $value;
}

function lower_then_stringish<T super class<C> as Stringish>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $string = lower_then_string(get_class_pointer());
  $string::method();
  $arraykey = lower_then_arraykey(get_class_pointer());
  $arraykey::method();
  $stringish = lower_then_stringish(get_class_pointer());
  $stringish::method();
}
