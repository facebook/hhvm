<?hh

class Baz {
  public function m() {}
  public static function sm() {}
}

function foo($x) {
  $ed = darray[];
  var_dump(HH\is_array_marked_legacy($ed));
  var_dump($ed);
  $ev = varray[];
  var_dump(HH\is_array_marked_legacy($ev));
  var_dump($ev);
  $a = darray['a' => $x];
  var_dump(HH\is_array_marked_legacy($a));
  var_dump($a);
  $b = varray[$x, $x];
  var_dump(HH\is_array_marked_legacy($b));
  var_dump($b);
  $c = darray['a' => 1];
  var_dump(HH\is_array_marked_legacy($c));
  var_dump($c);
  $d = varray[1, 2];
  var_dump(HH\is_array_marked_legacy($d));
  var_dump($d);
  $e = darray[1 => $x];
  var_dump(HH\is_array_marked_legacy($e));
  var_dump($e);
  $obj = new Baz();
  $g = inst_meth($obj, "m");
  var_dump(HH\is_array_marked_legacy($g));
  var_dump($g);

  // real vecs/dicts are not marked
  $a = dict['a' => $x];
  var_dump(HH\is_array_marked_legacy($a));
  var_dump($a);
  $b = vec[$x, $x];
  var_dump(HH\is_array_marked_legacy($b));
  var_dump($b);
  $c = dict['a' => 1];
  var_dump(HH\is_array_marked_legacy($c));
  var_dump($c);
  $d = vec[1, 2];
  var_dump(HH\is_array_marked_legacy($d));
  var_dump($d);
  $e = dict[1 => $x];
  var_dump(HH\is_array_marked_legacy($e));
  var_dump($e);

  // casts to d/varray
  $a = darray[];
  var_dump(HH\is_array_marked_legacy($a));
  var_dump($a);
  $b = darray(vec[1, 2]);
  var_dump(HH\is_array_marked_legacy($b));
  var_dump($b);
  $c = varray(dict[]);
  var_dump(HH\is_array_marked_legacy($c));
  var_dump($c);
  $d = varray(dict['k' => 1, 2 => 2]);
  var_dump(HH\is_array_marked_legacy($d));
  var_dump($d);
}

function bar<reify T>(vec<T> $v, int $i) {
  var_dump($v[$i]);
}

<<__EntryPoint>>
function main() {
  foo(1);
  bar<int>(vec[1, 2], 0);
  var_dump(gettype(varray[]));
  var_dump(gettype(darray['a' => 10]));
}
