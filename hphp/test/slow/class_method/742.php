<?hh

class A {
  <<__DynamicallyCallable>> function f() {
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
