<?hh

function foo($i) :mixed{
   var_dump($i);
   return 'a';
 }


 <<__EntryPoint>>
function main_1504() :mixed{
  $a1 = darray['a' => darray[]];
  $a1[foo(2)][foo(3)] = foo(4);
}
