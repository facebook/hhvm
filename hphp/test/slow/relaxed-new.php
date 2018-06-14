<?hh

class C {
  function m() { echo "done.\n"; }
}

function f() {
  new C()->m();
}

f();
