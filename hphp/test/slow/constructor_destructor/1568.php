<?hh

class parent_c {
  public function __construct() {
    echo "parent__construct";
  }
}
class child_c extends parent_c {
  public function __construct() {
    echo "child__construct";
  }
}

<<__EntryPoint>>
function main_1568() :mixed{
$v = new child_c;
unset($v);
}
