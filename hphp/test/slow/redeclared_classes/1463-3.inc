<?hh

  class child3 extends child2 {
    public $child3Val = 'child3';
    public $childProp = 'IamChild3';
    static $child3Static = 'child3Stat';
    function childMeth() {
      return 'I am child3';
    }
    static function child3StatMeth() {
      return 'Child 3 static method';
    }
    function parentChildMeth() {
      return parent::childMeth();
    }
  }
