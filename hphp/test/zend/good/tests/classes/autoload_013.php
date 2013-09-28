<?php
  function __autoload($name)
  {
      echo "In autoload: ";
      var_dump($name);
  }
  
  try {
      new ReflectionClass("UndefC");
  }
  catch (ReflectionException $e) {
      echo $e->getMessage();
  }
?>