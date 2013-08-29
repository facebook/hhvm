<?php
function autoload($className) { 

    $path = "./class_". str_replace("Core","",$className). ".php";

    if (is_file($path)) {
        require $path;
	
    }
}
function __autoload($class_name) {
    include "./class_".$class_name . '.php';
}
$a= new auto;
spl_autoload_register('autoload');
spl_autoload_unregister('autoload');
$b = new testCore;
var_dump(class_exists('test',false)); 
?>
