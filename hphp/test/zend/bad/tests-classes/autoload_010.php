<?php
  function __autoload($name)
  {
      echo "In autoload: ";
      var_dump($name);
  }
  
  class C implements UndefI
  {
  }
?>