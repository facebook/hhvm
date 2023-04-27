<?hh

function takes_container(Container<supportdyn<mixed>> $kc) : void {}

function takes_kcontainer(KeyedContainer<arraykey, supportdyn<mixed>> $kc)
  : void {}

function takes_trav(Traversable<supportdyn<mixed>> $kc) : void {}

function takes_ktrav(KeyedTraversable<arraykey, supportdyn<mixed>> $kc)
  : void {}

function f(supportdyn<vec<mixed>> $m,
           supportdyn<KeyedContainer<arraykey, mixed>> $c,
           supportdyn<Iterator<mixed>> $i)
       : vec<supportdyn<mixed>> {
  takes_container($m);
  takes_trav($m);
  takes_container($c);
  takes_trav($c);
  takes_trav($i);
  return $m;
}

function g(supportdyn<dict<arraykey, mixed>> $m,
           supportdyn<KeyedContainer<arraykey, mixed>> $kc,
           supportdyn<KeyedIterator<arraykey, mixed>> $ki) :
  dict<arraykey, supportdyn<mixed>> {
  takes_kcontainer($m);
  takes_kcontainer($kc);
  takes_ktrav($m);
  takes_ktrav($kc);
  takes_ktrav($ki);
  return $m;
}
