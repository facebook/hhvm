<?php
$oldcwd = getcwd();
chdir(dirname(__FILE__));
if (substr(PHP_OS, 0, 3) == 'WIN') {
	set_include_path(dirname(__FILE__).'/bug39542;.');
} else {
	set_include_path(dirname(__FILE__).'/bug39542:.');
}

function __autoload($class) {
    if (!require_once($class.'.php')) {
        error_log('Error: Autoload class: '.$class.' not found!');
    }
}

new bug39542();

chdir($oldcwd);
?>