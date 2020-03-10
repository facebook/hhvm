<?hh

function install_error_handler(): void {
  set_error_handler(($n, $str) ==> {
    if ($n === E_RECOVERABLE_ERROR) {
      print "Caught: ".$str."\n";
      return true;
    }
    return false;
  });
}
function CM($c, $m) { return __hhvm_intrinsics\create_clsmeth_pointer($c, $m); }
class A { static public function func1() { return 1; } }

function test_clsmeth_as_param($x) { var_dump($x); }
function test_varray_as_param(varray $x) { var_dump($x); }
function test_vdarray_as_param(varray_or_darray $x) { var_dump($x); }
function test_vec_as_param(vec $x) { var_dump($x); }
function test_vec_dict_as_param(vec_or_dict $x) { var_dump($x); }
function test_container_as_param(HH\Container $x) { var_dump($x); }

function test_soft_type_array_as_param(@array $x) { var_dump($x); }
function test_soft_type_varray_as_param(@varray $x) { var_dump($x); }

function test_param_type_cast($c, $f) {
  test_clsmeth_as_param(CM($c, $f));
  test_varray_as_param(CM($c, $f));
  test_vdarray_as_param(CM($c, $f));
  test_vec_as_param(CM($c, $f));
  test_vec_dict_as_param(CM($c, $f));
  test_container_as_param(CM($c, $f));
  test_soft_type_array_as_param(CM($c, $f));
  test_soft_type_varray_as_param(CM($c, $f));
}

function test_clsmeth_as_io_param(inout $x) { var_dump($x); }
function test_varry_as_io_param(inout varray $x) { var_dump($x); }
function test_vdarray_as_io_param(inout varray_or_darray $x) { var_dump($x); }
function test_vec_as_io_param(inout vec $x) { var_dump($x); }
function test_vec_dict_as_io_param(inout vec_or_dict $x) { var_dump($x); }
function test_container_as_io_param(inout HH\Container $x) { var_dump($x); }

function test_soft_type_array_as_io_param(inout @array $x) { var_dump($x); }
function test_soft_type_varry_as_io_param(inout @varray $x) { var_dump($x); }

function test_inout_param_type_cast($c, $f) {
  $x = CM($c, $f); test_clsmeth_as_io_param(inout $x);
  $x = CM($c, $f); test_varry_as_io_param(inout $x);
  $x = CM($c, $f); test_vdarray_as_io_param(inout $x);
  $x = CM($c, $f); test_vec_as_io_param(inout $x);
  $x = CM($c, $f); test_vec_dict_as_io_param(inout $x);
  $x = CM($c, $f); test_container_as_io_param(inout $x);
  $x = CM($c, $f); test_soft_type_array_as_io_param(inout $x);
  $x = CM($c, $f); test_soft_type_varry_as_io_param(inout $x);
}

function test_return_as_clsmeth() {
  return CM(A::class, 'func1');
}
function test_return_as_array() : array {
  return CM(A::class, 'func1');
}
function test_return_as_varray() : varray {
  return CM(A::class, 'func1');
}
function test_return_as_vdarray() : varray_or_darray {
  return CM(A::class, 'func1');
}
function test_return_as_vec() : vec {
  return CM(A::class, 'func1');
}
function test_return_as_vec_dict() : vec_or_dict {
  return CM(A::class, 'func1');
}
function test_return_as_container() : HH\Container {
  return CM(A::class, 'func1');
}

function test_return_type_cast() {
  var_dump(test_return_as_clsmeth());
  var_dump(test_return_as_array());
  var_dump(test_return_as_varray());
  var_dump(test_return_as_vdarray());
  var_dump(test_return_as_vec());
  var_dump(test_return_as_vec_dict());
  var_dump(test_return_as_container());
}

class B {
  public array $a;
  public varray $v_a;
  public varray_or_darray $v_d_a;
  public vec $vec_a;
  public vec_or_dict $vec_dict_a;
  public HH\Container $container_a;

  public static array $s_a;
  public static varray $s_v_a;
  public static varray_or_darray $s_v_d_a;
  public static vec $s_vec_a;
  public static vec_or_dict $s_vec_dict_a;
  public static HH\Container $s_container_a;
}

function test_class_prop_type_check($c, $f) {
  B::$s_a = CM($c, $f);
  B::$s_v_a = CM($c, $f);
  B::$s_v_d_a = CM($c, $f);
  B::$s_vec_a = CM($c, $f);
  B::$s_vec_dict_a = CM($c, $f);
  B::$s_container_a = CM($c, $f);
  var_dump(B::$s_a, B::$s_v_a, B::$s_v_d_a, B::$s_vec_a, B::$s_vec_dict_a,
           B::$s_container_a);

  $v = new B();
  $v->a = CM($c, $f);
  $v->v_a = CM($c, $f);
  $v->v_d_a = CM($c, $f);
  $v->vec_a = CM($c, $f);
  $v->vec_dict_a = CM($c, $f);
  $v->container_a = CM($c, $f);
  var_dump($v);
}

<<__EntryPoint>> function main(): void {
  install_error_handler();
  $c = A::class;
  $f = 'func1';
  test_param_type_cast($c, $f);
  test_inout_param_type_cast($c, $f);
  test_return_type_cast(CM($c, $f));
  test_class_prop_type_check($c, $f);
}
