<?php
  function __autoload($name)
  {
      echo "In autoload: ";
      var_dump($name);
  }

  $rc = new ReflectionClass("stdClass");

  try {
    $rc->implementsInterface("UndefI");
  } catch (ReflectionException $e) {
    echo $e->getMessage();
  }
?>