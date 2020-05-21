<?hh

class C {
  final public function f() {}
}
class D extends C {
  public function f() {}
}
<<__EntryPoint>>
function entrypoint_final_method(): void {
  print "Test\n";
}
