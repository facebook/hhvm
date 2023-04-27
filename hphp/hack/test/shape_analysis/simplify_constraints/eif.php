<?hh

function f(): void {
  $d = dict[];
  24 == 42 ? $d['a'] : $d['b'];
  inspect($d);
}
