<?hh

function handle_error($_errno, $str, ...) :mixed{
  echo " (error: $str)";
  return true;
}

class Foo { static function bar() :mixed{} }
class K { const A = 0; const B = 1; }
function LV($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }
function C(bool $b) :mixed{
  echo ($b ? " True\n" : " False\n");
}

function is_functions_static() :mixed{
  $m = Foo::bar<>;

  echo 'is_array($m):'        ;C(is_array($m));
  echo 'HH\is_any_array($m):' ;C(HH\is_any_array($m));
  echo 'HH\is_varray($m):'       ;C(HH\is_varray($m));
  echo 'HH\is_darray($m):'       ;C(HH\is_darray($m));
  echo 'HH\is_vec_or_varray($m):'       ;C(HH\is_vec_or_varray($m));
  echo 'HH\is_dict_or_darray($m):'       ;C(HH\is_dict_or_darray($m));
  echo '$m is arraylike:'     ;C($m is arraylike);
  echo '$m is shape(...):'    ;C($m is shape(...));
  echo '$m is shape(str,str):';C($m is shape(K::A=>string, K::B=>string));

  echo '$m is HH\Traversable:'        ;C($m is HH\Traversable);
  echo '$m is Traversable:'           ;C($m is Traversable);
  echo '$m is KeyedTraversable:'      ;C($m is KeyedTraversable);
  echo '$m is Container:'             ;C($m is Container);
  echo '$m is KeyedContainer:'        ;C($m is KeyedContainer);

  echo 'is_a($m, HH\Traversable):'        ;C(is_a($m, 'HH\Traversable'));
  echo 'is_a($m, Traversable):'           ;C(is_a($m, 'Traversable'));
  echo 'is_a($m, KeyedTraversable):'      ;C(is_a($m, 'KeyedTraversable'));
  echo 'is_a($m, Container):'             ;C(is_a($m, 'Container'));
  echo 'is_a($m, KeyedContainer):'        ;C(is_a($m, 'KeyedContainer'));
}

function is_functions_dynamic() :mixed{
  $m = LV(Foo::bar<>);

  echo 'is_array($m):'        ;C(is_array($m));
  echo 'HH\is_any_array($m):' ;C(HH\is_any_array($m));
  echo 'HH\is_varray($m):'       ;C(HH\is_varray($m));
  echo 'HH\is_darray($m):'       ;C(HH\is_darray($m));
  echo 'HH\is_vec_or_varray($m):'       ;C(HH\is_vec_or_varray($m));
  echo 'HH\is_dict_or_darray($m):'       ;C(HH\is_dict_or_darray($m));
  echo '$m is arraylike:'     ;C($m is arraylike);
  echo '$m is shape(...):'    ;C($m is shape(...));
  echo '$m is shape(str,str):';C($m is shape(K::A=>string, K::B=>string));

  echo '$m is HH\Traversable:'        ;C($m is HH\Traversable);
  echo '$m is Traversable:'           ;C($m is Traversable);
  echo '$m is KeyedTraversable:'      ;C($m is KeyedTraversable);
  echo '$m is Container:'             ;C($m is Container);
  echo '$m is KeyedContainer:'        ;C($m is KeyedContainer);

  echo 'is_a($m, HH\Traversable):'   ;C(LV('is_a')($m, 'HH\Traversable'));
  echo 'is_a($m, Traversable):'      ;C(LV('is_a')($m, 'Traversable'));
  echo 'is_a($m, KeyedTraversable):' ;C(LV('is_a')($m, 'KeyedTraversable'));
  echo 'is_a($m, Container):'        ;C(LV('is_a')($m, 'Container'));
  echo 'is_a($m, KeyedContainer):'   ;C(LV('is_a')($m, 'KeyedContainer'));
}

function is_functions_builtins() :mixed{
  $m = LV(Foo::bar<>);

  echo 'is_array($m):'       ;C(LV('is_array')($m));
  echo 'HH\is_any_array($m):';C(LV('HH\is_any_array')($m));
  echo 'HH\is_varray($m):'   ;C(LV('HH\is_varray')($m));
  echo 'HH\is_vec_or_varray($m):'   ;C(LV('HH\is_vec_or_varray')($m));
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

  is_functions_static();
  is_functions_dynamic();
  is_functions_builtins();
}
