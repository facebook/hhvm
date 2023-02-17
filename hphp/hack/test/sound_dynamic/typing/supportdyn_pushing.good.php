<?hh

function takes_container(Container<supportdyn<mixed>> $kc) : void {}

function takes_kcontainer(KeyedContainer<arraykey, supportdyn<mixed>> $kc)
  : void {}

function f(supportdyn<vec<mixed>> $m) : vec<supportdyn<mixed>> {
  takes_container($m);
  return $m;
}

function g(supportdyn<dict<arraykey, mixed>> $m) :
  dict<arraykey, supportdyn<mixed>> {
  takes_kcontainer($m);
  return $m;
}
