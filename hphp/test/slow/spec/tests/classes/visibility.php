<?hh

class C
{
// constants

    const CON1 = 123;           // implicitly static, and can't say so explicitly
//  public const CON2 = 123;    // class constants are implicitly public; can't say explicitly
//  protected const CON3 = 123; // class constants are implicitly public
//  private const CON4 = 123;   // class constants are implicitly public

// properties

//  $prop1a;            // rejected; must have a modifier of some sort
//  $prop1b = 123;      // rejected; must have a modifier of some sort
    public $prop2;
    protected $prop3;
    private $prop4;

    public static $sprop1;
    public static $sprop2;
    static protected $sprop3;   // visibility and static ordering unimportant
    private static $sprop4;

// methods

    function f1() :mixed{}
    public function f2() :mixed{}
    protected function f3() :mixed{}
    private function f4() :mixed{}

    static function sf1() :mixed{}
    public static function sf2() :mixed{}
    static protected function sf3() :mixed{}  // visibility and static ordering unimportant
    private static function sf4() :mixed{}

// constructors

    function __construct()[] {}               // OK on its own; implicitly public
//  public function __construct()[] {}        // OK on its own
//  protected function __construct()[] {}     // OK on its own
//  private function __construct()[] {}       // OK on its own

//  static function __construct()[] {}            // constructors can't be static
//  public static function __construct()[] {}
//  protected static function __construct()[] {}
//  private static function __construct()[] {}
}

abstract class D1
{
    public abstract function paf1($p1):mixed;
    abstract protected  function paf2():mixed;
//  private abstract function paf3();   // can't ever provide an implementation
    public static abstract  function pasf1():mixed;
    protected abstract static function pasf2($p1):mixed;
}

class D2 extends D1
{
//  public function paf1() {}           // Declaration of D2::paf1() must be compatible with D1::paf1($p1)
    public function paf1($q1) :mixed{}        // OK; has same visibility as abstract decl, and same signature
//  public function paf1($q1, $q2) {}   // Declaration of D2::paf1() must be compatible with D1::paf1($p1)
//  protected function paf1($q1) {}     // Access level to D2::paf1() must be public
//  private function paf1($q1) {}       // Access level to D2::paf1() must be public

//  public function paf2() {}           // OK; has wider visibility than abstract decl
    protected function paf2() :mixed{}        // OK; has same visibility as abstract decl
//  private function paf2() {}          // Access level to D2::paf2() must be protected

    public static function pasf1() :mixed{}   // OK; has same visibility as abstract decl
//  protected static function pasf1() {}// Access level to D2::pasf1() must be public
//  private static function pasf1() {}  // Access level to D2::pasf1() must be public

//  static public function pasf2($q1) {}    // OK; has wider visibility than abstract decl
//  static protected function pasf2() {}    // Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
    static protected function pasf2($q1) :mixed{} // OK; has same visibility as abstract decl, and same signature
//  static protected function pasf2($q1, $q2) {}    // Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
//  static private function pasf2() {$q1}   // Access level to D2::pasf2() must be protected
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  echo "CON1: " . C::CON1 . "\n"; // use :: notation, as a const is implicitly static

  $c = new C;     // calls public constructor
  $c->prop2;      // accesses public instance prop
}
