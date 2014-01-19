<?php
  function __autoload($name)
  {
      echo "In autoload: ";
      var_dump($name);
  }
  
  function f(UndefClass $x)
  {
  }
  f(new stdClass);
?>