<?hh

class X { public static $y = 2; }
function k() :mixed{ var_dump(is_int(X::$y)); }
function set_ref(inout $ref, $val) :mixed{ $ref = $val; }

<<__EntryPoint>>
function main_public_static_props_011() :mixed{
  $y = X::$y;
  set_ref(inout $y, 'asd');
  X::$y = $y;
  k();
}
