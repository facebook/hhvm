<?hh

class C {
}

<<__NEVER_INLINE>>
function getcl() {
  $a = new C();
  $b = new C();
  $c = new C();
  $d = new C();
  $e = new C();
  $f = new C();
  $g = new C();
  $h = new C();
  $i = new C();
  $j = new C();
  $k = new C();
  $l = new C();
  $m = new C();

  return () ==> {
    return vec[$a, $b, $c, $d, $e, $f, $g, $h, $i, $j, $k, $l, $m];
  };
}

<<__EntryPoint>>
function main() {
  $cl = getcl();
  var_dump($cl());
  var_dump($cl());
}
