<?hh

abstract class A<reify T as num> {
  public function f(): T { return 4; }
}
class C<reify Tc super int as num> extends A<Tc> {
  public function g(): void {
    $a = $this->f(); // runtime enforced Tc
  }
}

<<__EntryPoint>>
function main() :mixed{
  $c =  new C<int>();
  $c->g();
  echo "done\n";
}
