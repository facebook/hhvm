<?hh

class C {
  function m() { echo "done.\n"; }
}

function f() {
  new C()->m();
}


<<__EntryPoint>>
function main_relaxed_new() {
f();
}
