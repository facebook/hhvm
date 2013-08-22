<?php
function autoload($className) { 
    $path = "./class_". str_replace("Core","",$className). ".php";
	$a= new auto;
	var_dump(class_exists('auto'));
    if (is_file($path)) {
        require $path;
	var_dump(class_exists('test',false)); 
    }
}
function __autoload($class_name) {
    include "./class_".$class_name . '.php';
}
spl_autoload_register('autoload');
$b = new testCore;
?>
