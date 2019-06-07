<?hh

class a extends base1 {
  const aconst = "firstA";
  const a1const = 0;
  static $astat = 1;
  static $a1stat = 1;
  function __construct() {
    echo "first def made
";
  }
  static function foo() {
    return 1;
  }
}
