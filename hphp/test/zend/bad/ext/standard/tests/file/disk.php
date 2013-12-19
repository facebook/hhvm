<?php
ini_set('precision', 14);


var_dump(disk_free_space());
var_dump(disk_total_space());

var_dump(disk_free_space(-1));
var_dump(disk_total_space(-1));

var_dump(disk_free_space("/"));
var_dump(disk_total_space("/"));

var_dump(disk_free_space("/some/path/here"));
var_dump(disk_total_space("/some/path/here"));

echo "Done\n";
?>