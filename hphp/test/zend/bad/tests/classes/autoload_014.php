<?php
  function __autoload($name)
  {
      echo "In autoload: ";
      var_dump($name);
  }
  
  try {
      new ReflectionMethod("UndefC::test");
  }
  catch (ReflectionException $e) {
      echo $e->getMessage();
  }
?>