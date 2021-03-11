<?hh
function my_generator() {
  $value = yield null;
  var_dump(coerce_to_hack_arrays(debug_backtrace()));
}

function my_wrapper() {
  $gen = my_generator();
  $gen->next();
  $gen->send(null);
}

class my_class {
  static function my_member_generator() {
    $value = yield null;
    var_dump(coerce_to_hack_arrays(debug_backtrace()));
  }
}

function my_class_wrapper() {
  $gen = my_class::my_member_generator();
  $gen->next();
  $gen->send(null);
}
<<__EntryPoint>>
function entrypoint_debug_backtrace_continuation(): void {

  my_wrapper();
  my_class_wrapper();
}

function coerce_to_hack_arrays($input) {
  if (HH\is_vec_or_varray($input)) {
    $result = vec[];
    foreach ($input as $v) {
      $result[] = coerce_to_hack_arrays($v);
    }
    return $result;
  } else if (HH\is_dict_or_darray($input)) {
    $result = dict[];
    foreach ($input as $k => $v) {
      $result[$k] = coerce_to_hack_arrays($v);
    }
    return $result;
  }
  return $input;
}
