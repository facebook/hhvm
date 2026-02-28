<?hh

class A {
 public $a;
 function __toString() :mixed{
 return $this->a;
}
}

 <<__EntryPoint>>
function main_173() :mixed{
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = vec[$a, $b];
 sort(inout $arr, SORT_REGULAR);
 print ((string)$arr[0]);
}
