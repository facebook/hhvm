<?hh

class A { const ctx C = [rx]; }

function defaults() :mixed{ echo "in defaults\n"; }
function rx()[rx]   :mixed{ echo "in rx\n"; }
function pure()[]   :mixed{ echo "in pure\n"; }

function defaults_r<reify T>() :mixed{ echo "in defaults\n"; }

function foo((function()[_]: void) $x)[ctx $x] :mixed{
  defaults();
  rx();
  pure();
}

function bar(A $x)[$x::C] :mixed{
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
function main() :mixed{
  bar(new A);
}
