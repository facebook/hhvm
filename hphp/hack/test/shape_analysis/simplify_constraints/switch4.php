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
      $d = dict[];
      $d['b'] = 'hi';
      inspect($d);
      // FALLTHROUGH
    default:
      $d['c'] = false;
      inspect($d);
  }
  $d['d'] = 3.14;
  inspect($d);
}

function inspect(mixed $_): void {}
