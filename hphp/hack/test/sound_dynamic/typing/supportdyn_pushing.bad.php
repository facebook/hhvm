<?hh

function takes_iter(Iterator<supportdyn<mixed>> $kc) : void {}

function takes_kiter(KeyedIterator<arraykey, supportdyn<mixed>> $kc)
  : void {}

function f(supportdyn<Iterator<mixed>> $i) : void {
  takes_iter($i);
}

function g(supportdyn<KeyedIterator<arraykey, mixed>> $ki) : void {
  takes_kiter($ki);
}
