<?php
  class Foo {
    public function __get($name) {
      return array('Hello', 'World');    
    }
  }
  
  $obj=new Foo();
  foreach($obj->arr as $value)
    echo "$value\n";
?>