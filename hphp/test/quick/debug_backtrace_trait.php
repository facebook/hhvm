<?hh
<<__EntryPoint>>
function entrypoint_debug_backtrace_trait(): void {

  include 'debug_backtrace_trait_helper.inc';
  (new C)->bar(new stdClass);
  (new C)->bar(12);
}
