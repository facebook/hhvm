<?php

class DB extends mysqli {

    private function __construct($hostname, $username, $password, $database) {
        var_dump("DB::__construct() called");
    }
}

$DB = new DB();

echo "Done\n";
?>
