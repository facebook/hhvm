<?php
  class aclass
  {
      const myConst = "hello";
  }
  
  echo "\nAttempting to create a reference to a class constant - should be parse error.\n";
  $a = &aclass::myConst;
?>