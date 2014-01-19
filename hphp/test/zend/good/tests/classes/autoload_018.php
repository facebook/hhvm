<?php
  function __autoload($name)
  {
      echo "IN:  " . __METHOD__ . "($name)\n";
      
      static $i = 0;
      if ($i++ > 10) {
          echo "-> Recursion detected - as expected.\n";
          return;
      }
      
      class_exists('UndefinedClass' . $i);
      
      echo "OUT: " . __METHOD__ . "($name)\n";
  }
  
  var_dump(class_exists('UndefinedClass0'));
?>