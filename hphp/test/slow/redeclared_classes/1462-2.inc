<?hh

class a extends base2 {
  const aconst = "secondA";
  const a2const = 0;
  public static $astat = 2;
  public static $a2stat = 2;
  function __construct() {
    echo "second def made
";
  }
  static function foo() {
    return 2;
  }
}
