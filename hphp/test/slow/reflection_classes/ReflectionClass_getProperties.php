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

trait t1 {
  private $t1Private;
  protected $t1Protected;
  public $t1Public;
  private static $t1PrivateStatic;
  protected static $t1ProtectedStatic;
  public static $t1PublicStatic;
}

trait t2 {
  private static $t2PrivateStatic;
  protected static $t2ProtectedStatic;
  public static $t2PublicStatic;
  private $t2Private;
  protected $t2Protected;
  public $t2Public;
}

trait t3 {
  private $t3Private;
  protected $t3Protected;
  public $t3Public;
  private static $t3PrivateStatic;
  protected static $t3ProtectedStatic;
  public static $t3PublicStatic;
}

class cls1 {
  use t1;
  private $cls1Private;
  protected $cls1Protected;
  public $cls1Public;
  private static $cls1PrivateStatic;
  protected static $cls1ProtectedStatic;
  public static $cls1PublicStatic;
}

class cls2 extends cls1 {
  use t2;
  private $cls1Private;
  protected $cls1Protected;
  public $cls1Public;
  private $t1Private;
  protected $t1Protected;
  public $t1Public;
  private $cls2Private;
  protected $cls2Protected;
  public $cls2Public;
  private static $cls2PrivateStatic;
  protected static $cls2ProtectedStatic;
  public static $cls2PublicStatic;
}

class cls3 extends cls2 {
  use t3;
  private static $cls2PrivateStatic;
  protected static $cls2ProtectedStatic;
  public static $cls2PublicStatic;
  private static $t2PrivateStatic;
  protected static $t2ProtectedStatic;
  public static $t2PublicStatic;
  private $cls3Private;
  protected $cls3Protected;
  public $cls3Public;
  private static $cls3PrivateStatic;
  protected static $cls3ProtectedStatic;
  public static $cls3PublicStatic;
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
properties_dump('cls1');
properties_dump('cls2');
properties_dump('cls3');
