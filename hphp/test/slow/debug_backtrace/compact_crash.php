<?hh

function profile($what, $fn) {
  if ($what == 'exit' && $fn == 'X::foo') {
    fb_setprofile(null);
    var_dump(debug_backtrace(0));
  }
}

class X {
  <<__NEVER_INLINE>>
  function foo() {
    var_dump('hello');
  }
}

function main() {
  fb_setprofile('profile');
  (new X)->foo();
}

main();
