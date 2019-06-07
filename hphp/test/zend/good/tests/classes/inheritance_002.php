<?hh
class Base_php5 {
  function __construct() {
    var_dump('Base constructor');
  }
  }

class Child_php5 extends Base_php5 {
  function __construct() {
    var_dump('Child constructor');
    parent::__construct();
  }
  }

<<__EntryPoint>>
function main() {
  echo "### PHP 5 style\n";
  $c5= new Child_php5();
}
