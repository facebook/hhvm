<?php

class privateBeforeEverythingClass {
  private $privateProperty;
  private static $privateStaticProperty;
  protected $protectedProperty;
  public $publicProperty;
  protected static $protectedStaticProperty;
  public static $publicStaticProperty;
}

class privateAfterEverythingClass {
  protected $protectedProperty;
  public $publicProperty;
  protected static $protectedStaticProperty;
  public static $publicStaticProperty;
  private $privateProperty;
  private static $privateStaticProperty;
}

class protectedBeforeEverythingClass {
  protected $protectedProperty;
  protected static $protectedStaticProperty;
  private $privateProperty;
  public $publicProperty;
  private static $privateStaticProperty;
  public static $publicStaticProperty;
}

class protectedAfterEverythingClass {
  private $privateProperty;
  public $publicProperty;
  private static $privateStaticProperty;
  public static $publicStaticProperty;
  protected $protectedProperty;
  protected static $protectedStaticProperty;
}

class publicBeforeEverythingClass {
  public $publicProperty;
  public static $publicStaticProperty;
  private $privateProperty;
  protected $protectedProperty;
  private static $privateStaticProperty;
  protected static $protectedStaticProperty;
}

class publicAfterEverythingClass {
  private $privateProperty;
  protected $protectedProperty;
  private static $privateStaticProperty;
  protected static $protectedStaticProperty;
  public $publicProperty;
  public static $publicStaticProperty;
}

class staticBeforeEverythingClass {
  private static $privateStaticProperty;
  protected static $protectedStaticProperty;
  public static $publicStaticProperty;
  private $privateProperty;
  protected $protectedProperty;
  public $publicProperty;
}

class staticAfterEverythingClass {
  private $privateProperty;
  protected $protectedProperty;
  public $publicProperty;
  private static $privateStaticProperty;
  protected static $protectedStaticProperty;
  public static $publicStaticProperty;
}

function properties_dump($class_name) {
  $reflectionClass = new ReflectionClass($class_name);
  var_dump(array_map(function($prop) { return $prop->getName(); },
                     $reflectionClass->getProperties()));
}

properties_dump('privateBeforeEverythingClass');
properties_dump('privateAfterEverythingClass');
properties_dump('protectedBeforeEverythingClass');
properties_dump('protectedAfterEverythingClass');
properties_dump('publicBeforeEverythingClass');
properties_dump('publicAfterEverythingClass');
properties_dump('staticBeforeEverythingClass');
properties_dump('staticAfterEverythingClass');
