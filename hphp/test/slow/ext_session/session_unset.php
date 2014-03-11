<?php

class dtor { public function __destruct() { echo "heyo\n"; } }
session_start();
echo "heh\n";
$_SESSION['asdasd'] = new dtor;
session_unset();
echo "after unset\n";
var_dump($_SESSION['asdasd']);
