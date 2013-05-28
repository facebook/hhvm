<?php

interface i1 {
}
 interface i2 {
}
class cls1 implements i1, i2 {
   const CLS_CONST = 2;
  protected $prop1;
  public static $prop2 = 23;
  function method1($param1) {
 print $param1;
}
 }
 function &func1(cls1 $p1, &$p2, $p3='def') {
   static $a=1;
 var_dump($p1);
}
 function func2($a) {
 var_dump($a);
}
 class cls2 extends cls1 {
}
function dump_func($func) {
  var_dump($func->getName());
   var_dump($func->isInternal());
   var_dump($func->isUserDefined());
   var_dump($func->isClosure());
   $vars = $func->getStaticVariables();
   var_dump(count($vars));
  var_dump(isset($vars['a']));
  var_dump($func->returnsReference());
   var_dump($func->getNumberOfParameters());
   var_dump($func->getNumberOfRequiredParameters());
   foreach ($func->getParameters() as $name => $param) {
    var_dump($name);
     dump_param($param);
   }
}
 function verify_class($cls) {
  if ($cls) {
    var_dump($cls->getName());
   }
 else {
    var_dump(null);
  }
}
function verify_classes($classes) {
  ksort($classes);
  foreach ($classes as $cls) {
    verify_class($cls);
   }
}
function dump_param($param) {
  var_dump($param->getName());
   var_dump($param->isPassedByReference());
   verify_class($param->getDeclaringClass());
   verify_class($param->getClass());
   var_dump($param->isArray());
   var_dump($param->allowsNull());
   var_dump($param->isOptional());
   var_dump($param->isDefaultValueAvailable());
   if ($param->isOptional()) {
   }
   var_dump($param->getPosition());
 }
 function dump_prop($prop, $obj) {
  var_dump($prop->getName());
   var_dump($prop->isPublic());
   var_dump($prop->isPrivate());
   var_dump($prop->isProtected());
   var_dump($prop->isStatic());
   var_dump($prop->getModifiers() & 0xffff);
   if ($prop->isPublic()) {
     var_dump($prop->getValue($obj));
     if (!$prop->isStatic()) {
      var_dump($prop->setValue($obj, 78));
     }
    var_dump($prop->getValue($obj));
   }
   verify_class($prop->getDeclaringClass());
 }
 function dump_class($cls, $obj) {
  var_dump($cls->isInstance($obj));
   var_dump($cls->getName());
   var_dump($cls->isInternal());
   var_dump($cls->isUserDefined());
   var_dump($cls->isInstantiable());
   var_dump($cls->hasConstant('CLS_CONST'));
   var_dump($cls->hasMethod('method1'));
   var_dump($cls->hasProperty('prop1'));
   dump_func($cls->getMethod('method1'));
   dump_prop($cls->getProperty('prop1'), $obj);
   verify_classes($cls->getInterfaces());
   var_dump($cls->isInterface());
   var_dump($cls->isAbstract());
   var_dump($cls->isFinal());
   var_dump($cls->getModifiers() & 0xffff);
   verify_class($cls->getParentClass());
   var_dump($cls->isSubclassOf('i1'));
   var_dump($cls->getStaticPropertyValue('prop2'));
   cls1::$prop2 = 45;
   var_dump($cls->getStaticPropertyValue('prop2'));
   var_dump(cls1::$prop2);
   var_dump($cls->isIterateable());
   var_dump($cls->implementsInterface('i2'));
   foreach ($cls->getProperties() as $name => $prop) {
    var_dump($name);
     dump_prop($prop, $obj);
   }
  foreach ($cls->getMethods() as $name => $func) {
    var_dump($name);
     dump_func($func);
     var_dump($func->isFinal());
     var_dump($func->isAbstract());
     var_dump($func->isPublic());
     var_dump($func->isPrivate());
     var_dump($func->isProtected());
     var_dump($func->isStatic());
     var_dump($func->isConstructor());
     var_dump($func->isDestructor());
     var_dump($func->getModifiers() & 0xFFFF);
     verify_class($func->getDeclaringClass());
     if ($name == 'method1') $func->invoke($obj, 'invoked');
   }
}
$func = new ReflectionFunction('func1');
 dump_func($func);
 $func = new ReflectionFunction('func2');
 $func->invoke('invoked');
$cls = new ReflectionClass('cls1');
 $obj = $cls->newInstance();
 dump_class($cls, $obj);
$cls = new ReflectionClass('cls2');
 $obj = $cls->newInstance();
 dump_class($cls, $obj);
