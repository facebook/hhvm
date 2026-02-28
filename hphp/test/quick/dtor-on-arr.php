<?hh

class C {
}

<<__EntryPoint>> function f() :mixed{
  if (vec[new C()]) {
    echo "branch works\n";
  } else {
    echo "branch broken\n";
  }
}
