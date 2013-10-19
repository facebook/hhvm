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

$privateProperty = new ReflectionProperty('testClass', 'privateProperty');
var_dump($privateProperty->getDocComment());
$protectedProperty = new ReflectionProperty('testClass', 'protectedProperty');
var_dump($protectedProperty->getDocComment());
$publicProperty = new ReflectionProperty('testClass', 'publicProperty');
var_dump($publicProperty->getDocComment());
$publicStaticProperty = new ReflectionProperty('testClass', 'publicStaticProperty');
var_dump($publicStaticProperty->getDocComment());
