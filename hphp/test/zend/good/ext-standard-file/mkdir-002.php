<?php

var_dump(mkdir("testdir", 0777));
var_dump(mkdir("testdir/subdir", 0777));
var_dump(`ls -l testdir`);
var_dump(rmdir("testdir/subdir"));
var_dump(rmdir("testdir"));

var_dump(mkdir("./testdir", 0777));
var_dump(mkdir("./testdir/subdir", 0777));
var_dump(`ls -l ./testdir`);
var_dump(rmdir("./testdir/subdir"));
var_dump(rmdir("./testdir"));

var_dump(mkdir(dirname(__FILE__)."/testdir", 0777));
var_dump(mkdir(dirname(__FILE__)."/testdir/subdir", 0777));
$dirname = dirname(__FILE__)."/testdir";
var_dump(`ls -l $dirname`);
var_dump(rmdir(dirname(__FILE__)."/testdir/subdir"));
var_dump(rmdir(dirname(__FILE__)."/testdir"));

echo "Done\n";
?>