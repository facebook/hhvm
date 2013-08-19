<?php
function autoload($className) { 

    $path = "./class_". str_replace("Core","",$className). ".php";

    if (is_file($path)) {
        require $path;
	var_dump(class_exists('test',false)); 
    }

}
spl_autoload_register('autoload');
$foo = new testCore;
?>
