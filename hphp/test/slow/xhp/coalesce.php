<?hh

class :foo {
  public function __toString()[] :mixed{
    return "foo";
  }
}


<<__EntryPoint>>
function main_coalesce() :mixed{
$a = null;
$b = $a ?? <foo />;
var_dump($b->__toString());
}
