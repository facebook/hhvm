<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with different class/object keywords
 *   scope related : 
 *     static - T_STATIC(346), global - T_GLOBAL(340),
 *     private - T_PRIVATE(343), public - T_PUBLIC(341),
 *     protected - T_PROTECTED(342)
 *   class/object related : 
 *     var - T_VAR(347), abstract - T_ABSTRACT(345), 
 *     interface - T_INTERFACE(353), class - T_CLASS(352),
 *     extends - T_EXTENDS(354), implements - T_IMPLEMENTS(355), new - T_NEW(299)
*/

echo "*** Testing token_get_all() : with class/object constructs ***\n";

$source = '<?php
interface MyInterface
{
  public const var $var = 10;
}
abstract class MyClass
{
  private var $a;
  public var $b;
  protected var $c;
  static $d;
  final $e = 10;
  
  abstract public function myFunction($a);
}
class ChildClass extends MyClass implements MyInterface
{
  global $value;
  function myFunction($a)
  {
    $a = new ChildClass();
    if($a instanceof MyClass)
      echo "object created";
  }
}
ChildClass::myFunction(10);
?>';
$tokens =  token_get_all($source);
var_dump($tokens);

echo "Done"
?>
