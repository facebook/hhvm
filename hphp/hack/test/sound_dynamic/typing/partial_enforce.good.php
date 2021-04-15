<?hh

function my_count(Container<mixed> $c) : int {
  return 0;
}
function my_idx<Tk as arraykey, Tv>(
  ?KeyedContainer<Tk, Tv> $collection,
  ?Tk $index
) : ?Tv {
  return null;
}

function expects_dyn(dynamic $d) : void {}
function expects_dict_dyn (dict<arraykey, dynamic> $d) : void {}
function expects_keyset (keyset<arraykey> $d) : void {}
function expects_vec_dyn (vec<dynamic> $v) : void {}
function expects_vec_mix (vec<mixed> $v) : void {}
function expects_vecT<T> (vec<T> $v) : T {
  return $v[0];
}
class C {}

function f(bool $b, dynamic $d) : dynamic {
  expects_dict_dyn($d);
  expects_keyset($d);
  expects_vec_dyn($d);
  expects_vec_mix($d);
  my_count($d);
  if ($b) {
    $x = expects_vecT($d);
  }
  else {
    $x = my_idx($d, 0);
  }
  return $x;
}
