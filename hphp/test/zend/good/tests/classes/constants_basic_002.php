<?php
  class aclass
  {
      const myConst = "hello";
  }
  
  echo "\nRead class constant.\n";
  var_dump(aclass::myConst);
  
  echo "\nFail to read class constant from instance.\n";
  $myInstance = new aclass();
  var_dump($myInstance->myConst);
  
  echo "\nClass constant not visible in object var_dump.\n";
  var_dump($myInstance)
?>