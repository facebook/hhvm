<?hh

function profile($what, $fn) {
  if ($what == 'exit' && $fn == 'X::foo') {
    fb_setprofile(null);
    var_dump(debug_backtrace(0));
  }
}

class X {
  function foo() {
    var_dump('hello');
  }
}

function main() {
  fb_setprofile('profile');
  (new X())->foo();
}

main();
