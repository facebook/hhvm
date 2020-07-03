<?hh

class Baz {
  public function m() {}
  public static function sm() {}
}

function foo($c) {
  $a = darray['a' => $c];
  var_dump(HH\is_array_marked_legacy($a));
  $b = varray[$c, $c];
  var_dump(HH\is_array_marked_legacy($b));
  $c = darray['a' => 1];
  var_dump(HH\is_array_marked_legacy($c));
  $d = varray[1, 2];
  var_dump(HH\is_array_marked_legacy($d));
  $e = darray[1 => $c];
  var_dump(HH\is_array_marked_legacy($e));
  $f = class_meth(Baz::class, "sm");
  var_dump(HH\is_array_marked_legacy($f));
  $obj = new Baz();
  $g = inst_meth($obj, "m");
  var_dump(HH\is_array_marked_legacy($g));

  // real vecs/dicts are not marked
  $a = dict['a' => $c];
  var_dump(HH\is_array_marked_legacy($a));
  $b = vec[$c, $c];
  var_dump(HH\is_array_marked_legacy($b));
  $c = dict['a' => 1];
  var_dump(HH\is_array_marked_legacy($c));
  $d = vec[1, 2];
  var_dump(HH\is_array_marked_legacy($d));
  $e = dict[1 => $c];
  var_dump(HH\is_array_marked_legacy($e));
}

function bar<reify T>(vec<T> $v, int $i) {
  var_dump($v[$i]);
}

<<__EntryPoint>>
function main() {
  foo(1);
  bar<int>(vec[1, 2], 0);
}
