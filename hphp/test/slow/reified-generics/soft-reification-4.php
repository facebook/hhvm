<?hh

class C<reify Ta, <<__Soft>> reify Tb> {
  public function f() :mixed{
    return 1 is Tb;
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<int, string>();
$c->f<int, string>();
}
