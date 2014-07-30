<?php

var_dump(mkdir("mkdir-002", 0777));
var_dump(mkdir("mkdir-002/subdir", 0777));
var_dump(`ls -l mkdir-002`);
var_dump(rmdir("mkdir-002/subdir"));
var_dump(rmdir("mkdir-002"));

var_dump(mkdir("./mkdir-002", 0777));
var_dump(mkdir("./mkdir-002/subdir", 0777));
var_dump(`ls -l ./mkdir-002`);
var_dump(rmdir("./mkdir-002/subdir"));
var_dump(rmdir("./mkdir-002"));

var_dump(mkdir(dirname(__FILE__)."/mkdir-002", 0777));
var_dump(mkdir(dirname(__FILE__)."/mkdir-002/subdir", 0777));
$dirname = dirname(__FILE__)."/mkdir-002";
var_dump(`ls -l $dirname`);
var_dump(rmdir(dirname(__FILE__)."/mkdir-002/subdir"));
var_dump(rmdir(dirname(__FILE__)."/mkdir-002"));

echo "Done\n";
?>