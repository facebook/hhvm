<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A<T> {}
class B<reify T> {}

function get_a(named A<int> $z, named B<int> $a): A<int> {
  return $z;
}

<<__EntryPoint>>
function main(): void {
  $get_a = (named A<int> $z, named B<int> $a): A<int> ==> $z;

  var_dump(get_class(get_a(a=new B<int>(), z=new A<int>())));
  var_dump(get_class($get_a(a=new B<int>(), z=new A<int>())));
}
