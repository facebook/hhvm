<?hh

function handle_error($_no, $str, ...) {
  if ($str === 'Implicit clsmeth to varray conversion' ||
      $str === 'Implicit clsmeth to vec conversion') {
    echo "** CAST: clsmeth -> varray\n";
    return true;
  }
  return false;
}

class Foo { static function bar() {} }
function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function array_get_static() {
  $m = class_meth(Foo::class, 'bar');

  var_dump($m[0], $m[1]);
  var_dump($m[LV(0)], $m[LV(1)]);

  var_dump($m[0][1], $m[1][2]);
  var_dump($m[LV(0)][1], $m[1][LV(2)]);

  var_dump(strlen($m[0]), strlen($m[1]));
  var_dump(strlen($m[LV(0)]), strlen($m[LV(1)]));

  var_dump(ord($m[0][0]), ord($m[1][1]));
  var_dump(ord($m[LV(0)][LV(0)]), ord($m[1][1]));

  var_dump($m[0] ^ $m[1]);
  var_dump($m[LV(0)] ^ $m[LV(1)]);
}

function array_get_dynamic() {
  $m = LV(class_meth(Foo::class, 'bar'));

  var_dump($m[0], $m[1]);
  var_dump($m[LV(0)], $m[LV(1)]);

  var_dump($m[0][1], $m[1][2]);
  var_dump($m[LV(0)][1], $m[1][LV(2)]);

  var_dump(strlen($m[0]), strlen($m[1]));
  var_dump(strlen($m[LV(0)]), strlen($m[LV(1)]));

  var_dump(ord($m[0][0]), ord($m[1][1]));
  var_dump(ord($m[LV(0)][LV(0)]), ord($m[1][1]));

  var_dump($m[0] ^ $m[1]);
  var_dump($m[LV(0)] ^ $m[LV(1)]);
}

function array_set_static() {
  $m = class_meth(Foo::class, 'bar');
  $m[0] = 'Alpha';
  var_dump($m);

  $m = class_meth(Foo::class, 'bar');
  $m[0][1] = 'x';
  var_dump($m);

  $m = class_meth(Foo::class, 'bar');
  $m[1] .= '-xyz';
  var_dump($m);
}

function array_set_dynamic() {
  $m = LV(class_meth(Foo::class, 'bar'));
  $m[0] = 'Alpha';
  var_dump($m);

  $m = LV(class_meth(Foo::class, 'bar'));
  $m[0][1] = 'x';
  var_dump($m);

  $m = LV(class_meth(Foo::class, 'bar'));
  $m[1] .= '-xyz';
  var_dump($m);
}

<<__EntryPoint>>
function main() {
  set_error_handler(handle_error<>);

  array_get_static();  array_get_static();  array_get_static();
  array_get_dynamic(); array_get_dynamic(); array_get_dynamic();

  // class_meth() doesn't support ElemD, these will fatal
  //array_set_static();  array_set_static();  array_set_static();
  //array_set_dynamic(); array_set_dynamic(); array_set_dynamic();
}
