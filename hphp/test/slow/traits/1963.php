<?hh

trait T1 {
  private function Func1() {
 echo "Hello";
 }
}
class C {
  use T1 {
    T1::Func1 as public;
  }
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->Func1();
}
