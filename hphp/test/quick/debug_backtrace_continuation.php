<?hh
function my_generator() {
  $value = yield null;
  var_dump(debug_backtrace());
}

function my_wrapper() {
  $gen = my_generator();
  $gen->next();
  $gen->send(null);
}

class my_class {
  static function my_member_generator() {
    $value = yield null;
    var_dump(debug_backtrace());
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
