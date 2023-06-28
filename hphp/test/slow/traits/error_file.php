<?hh

function foo() :mixed{
  var_dump(hphp_debug_caller_info());
}

<<__EntryPoint>> function main(): void {
  require "error_file.inc2";
  require "error_file.inc";
  require "error_file-class.inc";

  (new X)->f();
  (new X)->g();
  (new X)->h();
}
