<?php
  function __autoload($name)
  {
      echo "in autoload: $name\n";
  }
  
  var_dump(unserialize('O:1:"C":0:{}'));
?>