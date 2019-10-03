<?hh

class X { static $y = 2; }
function k() { var_dump(is_int(X::$y)); }
function set_ref(inout $ref, $val) { $ref = $val; }

<<__EntryPoint>>
function main_public_static_props_011() {
  $y = X::$y;
  set_ref(inout $y, 'asd');
  X::$y = $y;
  k();
}
