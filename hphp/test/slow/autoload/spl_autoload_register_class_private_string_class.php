<?php

    class ClassAutoloader {
        public function __construct() {
            spl_autoload_register('ClassAutoloader::loader');
        }
        private function loader($className) {
            echo 'Trying to load ', $className, ' via ', __METHOD__, "()\n";
            include './class_'.str_replace("Core","",$className) . '.php';
        }
    }

    $autoloader = new ClassAutoloader();

    $obj = new testCore();

?>
