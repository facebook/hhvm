<?hh

function f(): void {
  $d = dict[];
  $i = 42;
  switch ($i) {
    case 0:
      $d['a'] = 42;
      inspect($d);
      break;
    case 1:
      $d['b'] = 'hi';
      inspect($d);
      break;
    default:
      $d['c'] = false;
      inspect($d);
  }
  inspect($d);
}

function inspect(mixed $_): void {}
