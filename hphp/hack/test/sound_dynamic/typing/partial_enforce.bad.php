<?hh

function expects_vec_int(vec<int> $v) : void {}
function expects_varray_int(varray<int> $v) : void {}
function expects_darray_int(darray<int, int> $v) : void {}
function expects_dict_int(dict<int, int> $v) : void {}
function expects_keyset_int(keyset<int> $v) : void {}
function expects_vecTnum<T as num>(vec<T> $v) : void {}
function expects_vecT<T>(vec<T> $v) : T { return $v[0]; }

function test(dynamic $d) : void {
  expects_vec_int($d);
  expects_varray_int($d);
  expects_darray_int($d);
  expects_dict_int($d);
  expects_keyset_int($d);
  expects_vecTnum($d);
  $x = expects_vecT($d);
  expects_vec_int($x);
  expects_vec_int(expects_vecT($d));
}
