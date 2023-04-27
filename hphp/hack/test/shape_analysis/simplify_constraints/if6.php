<?hh

function f(): void {
  $d = dict[];
  if (42 === 24) {
    $d['a'];
  }
  inspect($d);
}
