<?hh

function expect_trav_sd(Traversable<supportdyn<mixed>> $m) : void {}
function expect_kc_sd(KeyedContainer<arraykey, supportdyn<mixed>> $m) : void {}
function expect_vec_sd(vec<supportdyn<mixed>> $m) : void {}

function f<T as supportdyn<mixed>>(supportdyn<mixed> $m, T $t) : void {
  if ($m is Traversable<_>) {
    expect_trav_sd($m);
  }
  else if ($m is KeyedContainer<_, _>) {
    expect_kc_sd($m);
  }
  else if ($m is vec<_>) {
    expect_vec_sd($m);
  }
  if ($t is Traversable<_>) {
    expect_trav_sd($t);
  }
  else if ($t is KeyedContainer<_, _>) {
    expect_kc_sd($t);
  }
  else if ($t is vec<_>) {
    expect_vec_sd($t);
  }
}
