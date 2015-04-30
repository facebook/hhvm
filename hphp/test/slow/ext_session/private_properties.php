<?php

class MyClass {
    private $id;
    public function __construct($id) {
        $this->id = $id;
    }
    public function getId() {
        return $this->id;
    }
}

$handler = new SessionHandler();
session_set_save_handler($handler);
session_start();
$_SESSION['a'] = new MyClass(12);

session_write_close();
unset($_SESSION);

session_start();
var_dump($_SESSION['a']);
