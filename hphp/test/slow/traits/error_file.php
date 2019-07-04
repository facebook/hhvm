<?hh

require "error_file.inc";

function foo() {
  var_dump(hphp_debug_caller_info());
}

class X {
  use T;
}
<<__EntryPoint>> function main(): void {
(new X)->f();
(new X)->g();
(new X)->h();
}
