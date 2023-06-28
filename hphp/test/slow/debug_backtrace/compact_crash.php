<?hh

function profile($what, $fn) :mixed{
  if ($what == 'exit' && $fn == 'X::foo') {
    fb_setprofile(null);
    var_dump(debug_backtrace(0));
  }
}

class X {
  <<__NEVER_INLINE>>
  function foo() :mixed{
    var_dump('hello');
  }
}
<<__EntryPoint>>
function main() :mixed{
  fb_setprofile(profile<>);
  (new X)->foo();
}
