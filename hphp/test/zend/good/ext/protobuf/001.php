<?php
/***************************************************************************
 * 
 * Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/

//error_reporting(E_ALL);
error_reporting(E_ERROR);
require 'test.inc';
class PHPUnit_Framework_TestCase {
  public function __construct() {
  
  }
  
  public function assertEquals($expect,$actual) {
    if ($expect != $actual) {
        echo "false\n";
    }
  }
}

class Test_Protobuf extends PHPUnit_Framework_TestCase {

  public function testParseEmbedded() {
    $embedded = new Foo();
    $embedded->setDoubleField(2.0);

    $expected = new Foo();
    $expected->setEmbeddedField($embedded);

    $actual = new Foo();
    $actual->parseFromString("\x8A\x1\x9\x9\x0\x0\x0\x0\x0\x0\x0\x40");

    $this->assertEquals($expected, $actual);
  }
     
  public function testParseRepeated() {
    $expected = new Foo();
    $expected->appendRepeatedField(2);
    $expected->appendRepeatedField(3);

    $actual = new Foo();
    $actual->parseFromString("\x80\x1\x2\x80\x1\x3");

    $this->assertEquals($expected, $actual);
  }
  
  public function parseField($field, $value, $packed)
  {
    $expected = new Foo();
    $setter = 'set' . ucfirst($field) . 'Field';
    $expected->{$setter}($value);

    $actual = new Foo();
    $actual->parseFromString($packed);

    $this->assertEquals($expected, $actual);
  }
  
  public function testParseSimple() {
    $this->parseField('double', 2.0, "\x9\x0\x0\x0\x0\x0\x0\x0\x40");
    $this->parseField('float', 3.0, "\x15\x0\x0\x40\x40");
    $this->parseField('int32', 4, "\x18\x4");
    $this->parseField('int64', 5, "\x20\x5");
    $this->parseField('uint32', 6, "\x28\x6");
    //$this->parseField('uint64', 7, "\x30\x7");
    $this->parseField('sint32', 8, "\x38\x10");
    $this->parseField('sint64', 9, "\x40\x12");
    $this->parseField('fixed32', 10, "\x4D\xA\x0\x0\x0");
    $this->parseField('fixed64', 11, "\x51\xB\x0\x0\x0\x0\x0\x0\x0");
    $this->parseField('sfixed32', 12, "\x5D\xC\x0\x0\x0");
    $this->parseField('sfixed64', 13, "\x61\xD\x0\x0\x0\x0\x0\x0\x0");
    $this->parseField('bool', true, "\x68\x1");
    $this->parseField('string', '15', "\x72\x2\x31\x35");
    $this->parseField('bytes', '16', "\x7A\x2\x31\x36");
  }
  
  public function testRepeatedFieldAccessors() {
    $empty = array();
    //empty
    $foo = new Foo();
    $this->assertEquals($foo->getRepeatedFieldCount(), 0);
    $this->assertEquals($foo->getRepeatedField(), $empty);
    $this->assertEquals($foo->getRepeatedFieldIterator()->getArrayCopy(), $empty);
    
    //two elements
    $foo->appendRepeatedField(2);
    $foo->appendRepeatedField(3);

    $this->assertEquals($foo->getRepeatedFieldCount(), 2);
    $this->assertEquals(array(2, 3), $foo->getRepeatedField());
    $this->assertEquals(array(2, 3), $foo->getRepeatedFieldIterator()->getArrayCopy());
      
      //clear
    $foo->clearRepeatedField();

    $this->assertEquals($foo->getRepeatedFieldCount(), 0);
    $this->assertEquals($empty, $foo->getRepeatedField());
    $this->assertEquals($empty, $foo->getRepeatedFieldIterator()->getArrayCopy());
  }
  
  public function testSerializeEmbedded() {
    $embedded = new Foo();
    $embedded->setDoubleField(2.0);

    $foo = new Foo();
    $foo->setEmbeddedField($embedded);
    
    $this->assertEquals($foo->serializeToString(), "\x8A\x1\x9\x9\x0\x0\x0\x0\x0\x0\x0\x40");
  }
  
  public function testSerializeError() {
    $bar = new Bar();

    try {
      $bar->serializeToString();
    } catch (Exception $ex) {
      $this->assertEquals(0, 0);
    }
  }
  
  public function testSerializeRepeated() {
    $foo = new Foo();
    $foo->appendRepeatedField(2);
    $foo->appendRepeatedField(3);

    $this->assertEquals($foo->serializeToString(), "\x80\x1\x2\x80\x1\x3");
  }
  
  public function serializeField($field, $value)
  {
    $foo = new Foo();
    $setter = 'set' . ucfirst($field) . 'Field';
    $foo->{$setter}($value);

    return $foo->serializeToString();
  }
  public function testSerializeSimple() {
    $this->assertEquals($this->serializeField('double', 2.0), "\x9\x0\x0\x0\x0\x0\x0\x0\x40");
    $this->assertEquals($this->serializeField('float', 3.0), "\x15\x0\x0\x40\x40");
    $this->assertEquals($this->serializeField('int32', 4), "\x18\x4");
    $this->assertEquals($this->serializeField('int64', 5), "\x20\x5");
    $this->assertEquals($this->serializeField('uint32', 6), "\x28\x6");
    //$this->assertEquals($this->serializeField('uint64', 7), "\x30\x7");
    $this->assertEquals($this->serializeField('sint32', 8), "\x38\x10");
    $this->assertEquals($this->serializeField('sint64', 9), "\x40\x12");
    $this->assertEquals($this->serializeField('fixed32', 10), "\x4D\xA\x0\x0\x0");
    $this->assertEquals($this->serializeField('fixed64', 11), "\x51\xB\x0\x0\x0\x0\x0\x0\x0");
    $this->assertEquals($this->serializeField('sfixed32', 12), "\x5D\xC\x0\x0\x0");
    $this->assertEquals($this->serializeField('sfixed64', 13), "\x61\xD\x0\x0\x0\x0\x0\x0\x0");
    $this->assertEquals($this->serializeField('bool', true), "\x68\x1");
    $this->assertEquals($this->serializeField('string', '15'), "\x72\x2\x31\x35");
    $this->assertEquals($this->serializeField('bytes', '16'), "\x7A\x2\x31\x36");
  }
  
  public function testSetFloatField() {
    $foo = new Foo();

    //from float type
    $foo->setDoubleField(2.0);
    $this->assertEquals($foo->getDoubleField(), 2.0);

    //from int type
    $foo->setDoubleField(3);
    $this->assertEquals($foo->getDoubleField(), 3);

    //from string type
    $foo->setDoubleField('4');
    $this->assertEquals($foo->getDoubleField(), 4);
  }
  
  public function testSetIntField() {
    $foo = new Foo();

    //from int type
    $foo->setInt32Field(2);
    $this->assertEquals($foo->getInt32Field(), 2);

    //from float type
    $foo->setInt32Field(3.0);
    $this->assertEquals($foo->getInt32Field(), 3);

    //from string type
    $foo->setInt32Field('4');
    $this->assertEquals($foo->getInt32Field(), 4);
  }
  
  public function testSetObjectValue() {
    $embedded = new Foo();
    $embedded->setDoubleField(2.0);

    $foo = new Foo();
    $foo->setEmbeddedField($embedded);

    $this->assertEquals($embedded, $foo->getEmbeddedField());
  }
    
  public function testSetStringField() {
    $foo = new Foo();

    //from string type
    $foo->setStringField('2');
    $this->assertEquals($foo->getStringField(), "2");

    //from int type
    $foo->setStringField(3);
    $this->assertEquals($foo->getStringField(), "3");

    //from float type
    $foo->setStringField(4.0);
    $this->assertEquals($foo->getStringField(), "4");
  }

  public function testUint64Field() {
    $foo = new Foo();
    $foo->setUint64Field('18446744073709551615');
    $packed = $foo->serializeToString();
    $foo->reset();
    $foo->parseFromString($packed);
    $this->assertEquals($foo->getUint64Field(), '18446744073709551615');
  }
}


function main () {
  $reflector = new ReflectionClass('Test_Protobuf');
  $methods = $reflector->getMethods();
  $test = new Test_Protobuf();
  
  foreach ($methods as $method) {
    $method_name = $method->name;
    $class_name = $method->class;
    if ($class_name != "PHPUnit_Framework_TestCase") {
      if(strstr($method_name,"test")) {
        $setter = $reflector->getMethod($method_name);
        $objValue = $setter->invoke($test);
      }            
    }
  }
  echo "true";
}

main();
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100 */
?>
