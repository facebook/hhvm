<?hh

class child1 extends base {
  public $child1Val = 'child1';
  public $childProp = 'IamChild1';
  public static $child1Static = 'child1Stat';
  function childMeth() {
    return 'I am child1';
  }
  static function child1StatMeth() {
    return 'Child 1 static method';
  }
  function parentChildMeth() {
    return parent::childMeth();
  }
}
