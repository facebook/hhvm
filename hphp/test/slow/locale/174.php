<?hh

class A {
 public $a;
 }

<<__EntryPoint>>
function main_174() :mixed{
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = vec[$b, $a];
print $arr[0]->a;
sort(inout $arr, SORT_REGULAR);
 print $arr[0]->a;
}
