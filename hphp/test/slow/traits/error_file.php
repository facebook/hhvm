<?hh

require "error_file.inc";

class X {
  use T;
}

(new X)->f();
(new X)->g();
(new X)->h();

function foo() {
  var_dump(hphp_debug_caller_info());
}
