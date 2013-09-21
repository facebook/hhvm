<?php
  function __autoload($name)
  {
      echo __FUNCTION__ . " $name\n";
      class_exists("undefinedCLASS");
  }
  
  class_exists("unDefinedClass");
?>