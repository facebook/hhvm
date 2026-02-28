<?hh

class stringer {
  public function __toString() :mixed{ throw new Exception("nope\n"); }
}
class dtor {}

function ignore() :mixed{}
function foo() :mixed{
  $x = new stringer();
  ignore(new dtor(),  "foo" . $x);
}
<<__EntryPoint>> function main(): void {
try {
  foo();
} catch (Exception $e) {
}

print "all ok\n";
}
