<?

class C {
  function __destruct() {
    echo "woot-cakes\n";
  }
}

function f() {
  if (array(new C())) {
    echo "branch works\n";
  } else {
    echo "branch broken\n";
  }
}

f();

