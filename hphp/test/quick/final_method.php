<?hh

class C {
  final public function f() :mixed{}
}
class D extends C {
  public function f() :mixed{}
}
<<__EntryPoint>>
function entrypoint_final_method(): void {
  print "Test\n";
}
