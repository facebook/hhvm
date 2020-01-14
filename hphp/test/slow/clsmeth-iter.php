<?hh

class Foo { static function bar() { echo "Bar!\n"; }}

function get_meth() {
  return __hhvm_intrinsics\launder_value(class_meth(Foo::class, 'bar'));
}

function iter_static() {
  $cm = class_meth(Foo::class, 'bar');
  foreach ($cm as $k => $v) {
    echo "iter_static(): $k -> $v\n";
  }
  $cm();
}

function iter_arg($arg) {
  foreach ($arg as $k => $v) {
    echo "iter_arg(): $k -> $v\n";
  }
  $arg();
}

function iter_dynamic() {
  iter_arg(get_meth());
  iter_arg([Foo::class, 'bar']);
  iter_arg(vec[Foo::class, 'bar']);
  iter_arg(dict[0 => Foo::class, 1 => 'bar']);
}

function iter_recursive($v) {
  foreach ($v as $k => $v) {
    echo "iter_recursive(): ".serialize($v)."\n";
    if (is_array($v)) {
      iter_recursive($v);
    }
  }
}

function iter_recursive_init() {
  iter_recursive([]);
  iter_recursive(['a' => ['b' => 'xyz']]);
  iter_recursive(['a' => ['b' => 'xyz'], 'f' => get_meth()]);
  iter_recursive(['a' => ['b' => get_meth()]]);
  iter_recursive(['a' => ['b' => class_meth(Foo::class, 'bar')]]);
  iter_recursive(class_meth(Foo::class, 'bar'));
}

function iter_no_liter() {
  foreach (get_meth() as $k => $v) {
    echo "iter_no_liter(): $k -> $v\n";
  }
}

<<__EntryPoint>>
function main() {
  iter_static(); iter_static(); iter_static();
  iter_static(); iter_static(); iter_static();
  iter_static(); iter_static(); iter_static();

  iter_dynamic(); iter_dynamic(); iter_dynamic();
  iter_dynamic(); iter_dynamic(); iter_dynamic();
  iter_dynamic(); iter_dynamic(); iter_dynamic();

  iter_recursive_init(); iter_recursive_init(); iter_recursive_init();
  iter_recursive_init(); iter_recursive_init(); iter_recursive_init();
  iter_recursive_init(); iter_recursive_init(); iter_recursive_init();

  iter_no_liter(); iter_no_liter(); iter_no_liter();
  iter_no_liter(); iter_no_liter(); iter_no_liter();
  iter_no_liter(); iter_no_liter(); iter_no_liter();
}
