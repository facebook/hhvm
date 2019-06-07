<?hh

class C {
}

<<__EntryPoint>> function f() {
  if (array(new C())) {
    echo "branch works\n";
  } else {
    echo "branch broken\n";
  }
}
