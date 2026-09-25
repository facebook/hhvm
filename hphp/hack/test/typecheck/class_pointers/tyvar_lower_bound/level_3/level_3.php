<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function id_arraykey<T as arraykey>(T $value): T {
  return $value;
}

function id_stringish<T as Stringish>(T $value): T {
  return $value;
}

function id_xhp_child<T as XHPChild>(T $value): T {
  return $value;
}

function id_nullable_string<T as ?string>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $arraykey = id_arraykey(get_class_pointer());
  $arraykey::method();
  $stringish = id_stringish(get_class_pointer());
  $stringish::method();
  $xhp_child = id_xhp_child(get_class_pointer());
  $xhp_child::method();
  $nullable_string = id_nullable_string(get_class_pointer());
  $nullable_string::method();
}
