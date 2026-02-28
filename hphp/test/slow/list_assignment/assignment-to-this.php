<?hh
class C {
  public function test() :mixed{
    list($this) = array(null);
  }
}
function main() :mixed{
  $c = new C();
  $c->test();
  echo "Done\n";
}

<<__EntryPoint>>
function main_assignment_to_this() :mixed{
main();
}
