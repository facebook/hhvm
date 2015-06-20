<?php

$temp_file = tempnam("/tmp", "obd_test");
file_put_contents($temp_file, "This is data");
// delimiter :
var_dump(ini_set("open_basedir",
                 "/home:/tmp:/invalid_root_dir_asdfasdf/dfg:dfg:"));
var_dump(ini_get("open_basedir"));
var_dump(file_get_contents($temp_file)); // this is fine
// this is fine since it is a subdir of an allowed dir
var_dump(ini_set("open_basedir", "/tmp/aasdfasdf:"));
var_dump(ini_get("open_basedir"));
// this is not fine any more since the file is not in the allowed dir set now
var_dump(file_get_contents($temp_file));
// this is not fine since it is not a subdir of an allowed dir
var_dump(ini_set("open_basedir", "/nonexistent:"));
var_dump(file_get_contents($temp_file)); // still not fine
var_dump(ini_get("open_basedir"));
