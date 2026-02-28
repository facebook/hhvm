<?hh
function a() :mixed{
    b();
}

function b() :mixed{
    c();
}

function c() :mixed{
    print_r(debug_backtrace(0, 1));
    print_r(debug_backtrace(0, 2));
    print_r(debug_backtrace(0, 0));
    print_r(debug_backtrace(0, 4));
}
<<__EntryPoint>>
function entrypoint_debug_backtrace_limit(): void {

  a();
}
