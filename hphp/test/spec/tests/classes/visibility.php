<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

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

    var $vprop1a;           // OK; implies public
    var $vprop1b = 123;     // OK; implies public
//  public var $vprop2;     // can't combine var with visibility modifiers
//  var public  $vprop2;
//  protected var $vprop3;
//  var protected  $vprop3;
//  var private $vprop4;

    static $sprop1;
    public static $sprop2;
    static protected $sprop3;   // visibility and static ordering unimportant
    private static $sprop4;

//  static var $vsprop1;    // can't combine var with static modifier
//  var static $vsprop1;
//  var public static $vsprop2;
//  protected var static $vsprop3;
//  private static var $vsprop4;

// methods
    
    function f1() {}
    public function f2() {}
    protected function f3() {}
    private function f4() {}

    static function sf1() {}
    public static function sf2() {}
    static protected function sf3() {}  // visibility and static ordering unimportant
    private static function sf4() {}

// constructors
    
    function __construct() {}               // OK on its own; implicitly public
//  public function __construct() {}        // OK on its own
//  protected function __construct() {}     // OK on its own
//  private function __construct() {}       // OK on its own

//  static function __construct() {}            // constructors can't be static
//  public static function __construct() {}
//  protected static function __construct() {}
//  private static function __construct() {}

// destructors
    
    function __destruct() {}                // OK on its own; implicitly public
//  public function __destruct() {}         // OK on its own
//  protected function __destruct() {}      // OK on its own
//  private function __destruct() {}        // OK on its own

//  static function __destruct() {}         // destructors can't be static
//  public static function __destruct() {}
//  protected static function __destruct() {}
//  private static function __destruct() {}
}

echo "CON1: " . C::CON1 . "\n"; // use :: notation, as a const is implicitly static

$c = new C;     // calls public constructor
$c->vprop1a;    // accesses public instance method
$c->vprop1b;    // accesses public instance method

abstract class D1
{
    public abstract function paf1($p1);
    abstract protected  function paf2();
//  private abstract function paf3();   // can't ever provide an implementation
    public static abstract  function pasf1();
    protected abstract static function pasf2($p1);
}

class D2 extends D1
{
//  public function paf1() {}           // Declaration of D2::paf1() must be compatible with D1::paf1($p1)
    public function paf1($q1) {}        // OK; has same visibility as abstract decl, and same signature
//  public function paf1($q1, $q2) {}   // Declaration of D2::paf1() must be compatible with D1::paf1($p1)
//  protected function paf1($q1) {}     // Access level to D2::paf1() must be public
//  private function paf1($q1) {}       // Access level to D2::paf1() must be public

//  public function paf2() {}           // OK; has wider visibility than abstract decl
    protected function paf2() {}        // OK; has same visibility as abstract decl
//  private function paf2() {}          // Access level to D2::paf2() must be protected

    public static function pasf1() {}   // OK; has same visibility as abstract decl
//  protected static function pasf1() {}// Access level to D2::pasf1() must be public
//  private static function pasf1() {}  // Access level to D2::pasf1() must be public

//  static public function pasf2($q1) {}    // OK; has wider visibility than abstract decl
//  static protected function pasf2() {}    // Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
    static protected function pasf2($q1) {} // OK; has same visibility as abstract decl, and same signature
//  static protected function pasf2($q1, $q2) {}    // Declaration of D2::pasf2() must be compatible with D1::pasf2($p1)
//  static private function pasf2() {$q1}   // Access level to D2::pasf2() must be protected
}
