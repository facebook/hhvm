<?hh

function add_to_keyset<Tk as arraykey, Tv as arraykey>(
  inout dict<Tk, keyset<Tv>> $dict,
  Tk $key,
  Tv $value,
)[]: bool {
  return false;
}

function f(string $k, int $v): void {
  $d = dict[];
  add_to_keyset(inout $d, $k, $v);
  // Need the key to be string, and not an expression dependent type with a string bound
  $d['a'];
}
