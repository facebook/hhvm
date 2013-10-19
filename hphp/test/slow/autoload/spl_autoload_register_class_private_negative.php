<?php

class ClassAutoloader {
  private function loader($className) {
    echo 'Trying to load ', $className, ' via ', __METHOD__, "()\n";
    if ($className == "testCore") {
      var_dump("myClass expect");
      test();
    }
  }
}

$autoloader = new ClassAutoloader();
try {
  spl_autoload_register(array($autoloader, 'loader'));
} catch (Exception $e) {
  echo "Caught exception.\n";
}
