<?hh

class C {
}

<<__EntryPoint>> function f() :mixed{
  if (varray[new C()]) {
    echo "branch works\n";
  } else {
    echo "branch broken\n";
  }
}
