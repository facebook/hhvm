<?php
class foo {
  public $bar = "ok";

  function method() { $this->yes = "done"; }
}

session_start();

session_decode("<wddxPacket version='1.0'><header/><data><struct><var name='data'><struct><var name='test1'><boolean value='true'/></var><var name='test2'><string>some string</string></var><var name='test3'><number>654321</number></var><var name='test4'><array length='3'><string>some string</string><boolean value='true'/><null/></array></var></struct></var><var name='class'><struct><var name='php_class_name'><string>foo</string></var><var name='bar'><string>ok</string></var><var name='yes'><string>done</string></var></struct></var></struct></data></wddxPacket>");

var_dump($_SESSION);

session_destroy();
