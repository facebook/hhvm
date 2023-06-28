<?hh

function profiler(...$args) :mixed{
  var_dump(debug_backtrace());
}

class C {
  function f<reify T>():mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  fb_setprofile(profiler<>);
  $c = new C();
  $c->f<int>();
}
