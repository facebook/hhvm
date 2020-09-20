<?hh

class A {
 public $a;
 function __toString() {
 return $this->a;
}
}

 <<__EntryPoint>>
function main_173() {
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = varray[$a, $b];
 sort(inout $arr, SORT_REGULAR);
 print ((string)$arr[0]);
}
