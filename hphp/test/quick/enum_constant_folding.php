<?hh

enum S: string {
  C = "A" . "B";
}

function f() {
  $a = S::C;
  print (S::isValid($a));
}

f();
