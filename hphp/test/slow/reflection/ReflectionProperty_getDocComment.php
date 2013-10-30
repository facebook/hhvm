<?php
class testClass
{
  /** private property */
  private $privateProperty;
  /** protected property */
  protected $protectedProperty;
  /** public property */
  public $publicProperty;
  /** public static property */
  public static $publicStaticProperty;
}

class childClass extends testClass
{
  /** child private property */
  private $privateProperty;
  /** child protected property */
  protected $protectedProperty;
  /** child public property */
  public $publicProperty;
  /** child public static property */
  public static $publicStaticProperty;
}

$privateProperty = new ReflectionProperty('testClass', 'privateProperty');
var_dump($privateProperty->getDeclaringClass()->getName());
var_dump($privateProperty->getDocComment());
$protectedProperty = new ReflectionProperty('testClass', 'protectedProperty');
var_dump($protectedProperty->getDeclaringClass()->getName());
var_dump($protectedProperty->getDocComment());
$publicProperty = new ReflectionProperty('testClass', 'publicProperty');
var_dump($publicProperty->getDeclaringClass()->getName());
var_dump($publicProperty->getDocComment());
$publicStaticProperty =
  new ReflectionProperty('testClass', 'publicStaticProperty');
var_dump($publicStaticProperty->getDeclaringClass()->getName());
var_dump($publicStaticProperty->getDocComment());

$privateProperty = new ReflectionProperty('childClass', 'privateProperty');
var_dump($privateProperty->getDeclaringClass()->getName());
var_dump($privateProperty->getDocComment());
$protectedProperty = new ReflectionProperty('childClass', 'protectedProperty');
var_dump($protectedProperty->getDeclaringClass()->getName());
var_dump($protectedProperty->getDocComment());
$publicProperty = new ReflectionProperty('childClass', 'publicProperty');
var_dump($publicProperty->getDeclaringClass()->getName());
var_dump($publicProperty->getDocComment());
$publicStaticProperty =
  new ReflectionProperty('childClass', 'publicStaticProperty');
var_dump($publicStaticProperty->getDeclaringClass()->getName());
var_dump($publicStaticProperty->getDocComment());
