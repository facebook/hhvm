<?php

class MyAutoLoader {

        static function autoLoad($className) {
        	echo __METHOD__ . "($className)\n";
        }
}

spl_autoload_register(array('MyAutoLoader', 'autoLoad'));

// and

$myAutoLoader = new MyAutoLoader();

spl_autoload_register(array($myAutoLoader, 'autoLoad'));

var_dump(spl_autoload_functions());

// check
var_dump(class_exists("TestClass", true));

?>
===DONE===
<?php exit(0); ?>