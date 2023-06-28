<?hh

class C {
  function m() :mixed{ echo "done.\n"; }
}

function f() :mixed{
  new C()->m();
}


<<__EntryPoint>>
function main_relaxed_new() :mixed{
f();
}
