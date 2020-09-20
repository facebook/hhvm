<?hh

class A {}
class B {}

function foo(): A {
  try {
    return new B;
  } finally {
    echo "finally\n";
  }
}


<<__EntryPoint>>
function main_finally_typecheck() {
var_dump(foo());
}
