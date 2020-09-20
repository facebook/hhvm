<?hh

  class base {
    public $baseVal =  'base';
    static $baseStatic = 'baseStat';
    function childProp() {
 return $this->childProp;
 }
    function testChildMeth() {
 return $this->childMeth();
 }
    static function baseStatMeth() {
      return 'Base static method';
    }
    function childMeth() {
 return 'I am base';
 }
  }
