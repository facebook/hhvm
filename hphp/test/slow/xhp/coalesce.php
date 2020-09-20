<?hh

class :foo {
  public function __toString() {
    return "foo";
  }
}


<<__EntryPoint>>
function main_coalesce() {
$a = null;
$b = $a ?? <foo />;
var_dump((string) $b);
}
