<?hh

class A {
   function f() {
 return "hello" ;
}
}

<<__EntryPoint>>
function main_742() {
;
 $g = new A();
 echo $g->{
'f'}
();
}
