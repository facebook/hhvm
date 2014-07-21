<?php
    error_reporting(-1);
    mkdir("a");
    file_put_contents("a/a.txt","test");
    var_dump(file_exists("a/b/../a.txt"));
?>
