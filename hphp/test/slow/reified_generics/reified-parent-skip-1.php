<?hh

class A<T> {}
class B<T> extends A<T> {}
class C<T1, reify T2> extends B<T1> {}

<<__EntryPoint>>
function main_async(): void {
  new C<string, int>();
  echo "ok\n";
}
