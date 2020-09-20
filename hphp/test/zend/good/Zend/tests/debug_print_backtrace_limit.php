<?hh
function a() {
    b();
}

function b() {
    c();
}

function c() {
    debug_print_backtrace(0, 1);
    debug_print_backtrace(0, 2);
    debug_print_backtrace(0, 0);
    debug_print_backtrace(0, 4);
}

<<__EntryPoint>>
function entrypoint_debug_print_backtrace_limit(): void {
  a();
}
