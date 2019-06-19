<?hh

function profiler(...$args) {
  var_dump(debug_backtrace());
}

class C {
  function f<reify T>(){}
}

<<__EntryPoint>>
function main() {
  fb_setprofile('profiler');
  $c = new C();
  $c->f<int>();
}
