<?hh

function lval(
  int $i,
  dict<string, string> $d1,
  dict<string, dict<string, int>> $d2,
  dynamic $d,
): void {
  ($i) += 42;
  ($d1)['foo'] = 'bar';
  ($d2['bar'])['baz'] = 13;
  ($d->bing) = 'crosby';
}
