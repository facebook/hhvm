<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function id_string<T as string>(T $value): T {
  return $value;
}

function id_unconstrained<T>(T $value): T {
  return $value;
}

function id_arraykey<T as arraykey>(T $value): T {
  return $value;
}

function id_stringish<T as Stringish>(T $value): T {
  return $value;
}

function id_multiple<T as string>(T $value): T where T as arraykey {
  return $value;
}

function lower_then_upper<T super class<C> as string>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(classname<C> $classname): void {
  $string = id_string(get_class_pointer());
  $string::method();
  $already_classname = id_string($classname);
  $already_classname::method();
  $unconstrained = id_unconstrained(get_class_pointer());
  $unconstrained::method();
  $multiple = id_multiple(get_class_pointer());
  $multiple::method();
  $upper_arrives = lower_then_upper(get_class_pointer());
  $upper_arrives::method();
  $arraykey = id_arraykey(get_class_pointer());
  $arraykey::method();
  $stringish = id_stringish(get_class_pointer());
  $stringish::method();
}

function test_supportdyn(supportdyn<class<C>> $class): void {
  $value = id_string($class);
  $value::method();
}

function test_union((class<C> | classname<C>) $value): void {
  $result = id_string($value);
  $result::method();
}

function test_intersection((class<C> & string) $value): void {
  $result = id_string($value);
  $result::method();
}
