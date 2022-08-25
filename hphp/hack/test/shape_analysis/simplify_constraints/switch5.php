<?hh

function f(): void {
  $d = dict[];
  switch (42) {
    case 0:
      $d['a'];
      break;
    case 1:
      $d['b1'];
      $d['b2'] = 42;
      break;
    default:
      $d['c1'] = 42;
      $d['c2'];
  }
  inspect($d);
}
