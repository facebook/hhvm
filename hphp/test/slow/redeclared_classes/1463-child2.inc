<?hh

class child2 extends child1 {
  public $child2Val = 'child2';
  public $childProp = 'IamChild2';
  static $child2Static = 'child2Stat';
  static function child2StatMeth() {
    return 'Child 2 static method';
  }
  function childMeth() {
    return 'I am child2';
  }
  function parentChildMeth() {
    return parent::childMeth();
  }
  function testChildMeth2() {
    return $this->childMeth();
  }
}
