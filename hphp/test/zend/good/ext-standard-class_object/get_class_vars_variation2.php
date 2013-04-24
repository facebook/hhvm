<?php
/* Prototype  : array get_class_vars(string class_name)
 * Description: Returns an array of default properties of the class. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

class Ancestor {
  function test() {
    var_dump(get_class_vars("Tester"));
  }
  
  static function testStatic() {
    var_dump(get_class_vars("Tester"));
  }
}

class Tester extends Ancestor {
  public $pub = "public var";
  protected $prot = "protected var";
  private $priv = "private var";
  
  static public $pubs = "public static var";
  static protected $prots = "protected static var";
  static private $privs = "private static var";
  
  function test() {
    var_dump(get_class_vars("Tester"));
  }
  
  static function testStatic() {
    var_dump(get_class_vars("Tester"));
  }
}

class Child extends Tester {
  function test() {
    var_dump(get_class_vars("Tester"));
  }
  
  static function testStatic() {
    var_dump(get_class_vars("Tester"));
  }
}

echo "*** Testing get_class_vars() : testing visibility\n";

echo "\n-- From global context --\n";
var_dump(get_class_vars("Tester"));

echo "\n-- From inside an object instance --\n";
$instance = new Tester();
$instance->test();

echo "\n-- From  a static context --\n";
Tester::testStatic();


echo "\n-- From inside an  parent object instance --\n";
$parent = new Ancestor();
$parent->test();

echo "\n-- From a parents static context --\n";
Ancestor::testStatic();


echo "\n-- From inside a child object instance --\n";
$child = new Child();
$child->test();

echo "\n-- From a child's static context --\n";
Child::testStatic();
?>
===DONE===