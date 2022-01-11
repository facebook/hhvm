<?hh

function f(): void {
  $x = dict[];
  $y = 42; // Irrelevant
  $z = $x;
  $z['a'];
  inspect($z);
}
