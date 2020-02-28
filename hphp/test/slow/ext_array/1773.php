<?hh

function xsort(inout $a) {
  $b = darray[];
  $b[0] = $a;
  uksort(inout $a, function ($i, $j) use($b) {
      if ($b[0][$i] == $b[0][$j]) return 0;
      return $b[0][$i] < $b[0][$j] ? -1 : 1;
    }
);
}
function test($x) {
  $a = varray[220,250,240,$x];
  xsort(inout $a);
  var_dump($a);
}

<<__EntryPoint>>
function main_1773() {
test(230);
}
