<?hh

enum S: string {
  C = "A" . "B";
}

<<__EntryPoint>> function f(): void {
  $a = S::C;
  print (S::isValid($a));
}
