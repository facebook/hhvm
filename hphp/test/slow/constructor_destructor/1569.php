<?hh

class parent_c {
  public function __construct() {
    echo "parent__construct";
  }
}
class child_c extends parent_c {
  public function __construct() {
    echo "child__construct";
    parent::__construct();
  }
}

<<__EntryPoint>>
function main_1569() :mixed{
$v = new child_c;
unset($v);
}
