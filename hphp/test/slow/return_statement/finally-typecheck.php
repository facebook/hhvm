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

var_dump(foo());
