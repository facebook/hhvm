<?hh

class ClassAutoloader {
  private function loader($className) {
    echo 'Trying to load ', $className, ' via ', __METHOD__, "()\n";
    if ($className == "testCore") {
      var_dump("myClass expect");
      test();
    }
  }
}


<<__EntryPoint>>
function main_spl_autoload_register_class_private_negative() {
$autoloader = new ClassAutoloader();
try {
  spl_autoload_register(varray[$autoloader, 'loader']);
} catch (Exception $e) {
  echo "Caught exception.\n";
}
}
