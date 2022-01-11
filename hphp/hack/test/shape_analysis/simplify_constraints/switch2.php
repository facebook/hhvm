<?hh

function f(): void {
  $i = 42;
  switch ($i) {
    case 0:
      $d = dict[];
      break;
    case 1:
      $d = dict['b' => 42];
      break;
    default:
      $d = dict['c' => true];
  }
  $d['a'] = 'hey';
  inspect($d);
}
