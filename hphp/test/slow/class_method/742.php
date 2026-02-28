<?hh

class A {
  <<__DynamicallyCallable>> function f() :mixed{
    return "hello" ;
  }
}

<<__EntryPoint>>
function main_742() :mixed{
;
 $g = new A();
 echo $g->{
'f'}
();
}
