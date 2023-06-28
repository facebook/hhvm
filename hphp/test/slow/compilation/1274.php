<?hh
 function test2($a) :mixed{
 var_dump($a);
 return 12345;
}
 class A extends B {
}
 class B {
 const CLASS_CONSTANT = 1;
}


<<__EntryPoint>>
function main_1274() :mixed{
$global = B::CLASS_CONSTANT;
 $another = test2($global);
}
