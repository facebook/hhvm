<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function id_nullable_string<T as ?string>(T $value): T {
  return $value;
}

function id_nullable_arraykey<T as ?arraykey>(T $value): T {
  return $value;
}

function id_nullable_stringish<T as ?Stringish>(T $value): T {
  return $value;
}

function id_nullable_xhp_child<T as ?XHPChild>(T $value): T {
  return $value;
}

function id_union<T as (string | float)>(T $value): T {
  return $value;
}

function get_class_pointer(): class<C> {
  return C::class;
}

function test(): void {
  $nullable_string = id_nullable_string(get_class_pointer());
  $nullable_string::method();
  $nullable_arraykey = id_nullable_arraykey(get_class_pointer());
  $nullable_arraykey::method();
  $nullable_stringish = id_nullable_stringish(get_class_pointer());
  $nullable_stringish::method();
  $nullable_xhp_child = id_nullable_xhp_child(get_class_pointer());
  $nullable_xhp_child::method();
  $union = id_union(get_class_pointer());
  $union::method();
}
