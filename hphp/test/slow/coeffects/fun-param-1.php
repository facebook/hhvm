<?hh

class A { const ctx C = [rx]; }

function defaults() { echo "in defaults\n"; }
function rx()[rx]   { echo "in rx\n"; }
function pure()[]   { echo "in pure\n"; }

function defaults_r<reify T>() { echo "in defaults\n"; }

function foo((function()[_]: void) $x)[ctx $x] {
  defaults();
  rx();
  pure();
}

function bar(A $x)[$x::C] {
  foo(() ==> {});
  foo(()[defaults] ==> {});
  foo(()[rx] ==> {});
  foo(()[pure] ==> {});

  foo(defaults<>);
  foo(rx<>);
  foo(pure<>);
  foo(defaults_r<int>);

  foo(null);
}

<<__EntryPoint>>
function main() {
  bar(new A);
}
