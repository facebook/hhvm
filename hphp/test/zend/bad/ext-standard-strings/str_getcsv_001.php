<?php

// string input[, string delimiter[, string enclosure[, string escape]]]
var_dump(str_getcsv('"f", "o", ""'));
print "-----\n";
var_dump(str_getcsv('foo||bar', '|'));
print "-----\n";
var_dump(str_getcsv('foo|bar', '|'));
print "-----\n";
var_dump(str_getcsv('|foo|-|bar|', '-', '|'));
print "-----\n";
var_dump(str_getcsv('|f.|.|bar|.|-|-.|', '.', '|', '-'));
print "-----\n";
var_dump(str_getcsv('.foo..bar.', '.', '.', '.'));
print "-----\n";
var_dump(str_getcsv('.foo. .bar.', '   ', '.', '.'));
print "-----\n";
var_dump(str_getcsv((binary)'1foo1 1bar111', (binary)'   ', (binary)'1   ', (binary) '\  '));
print "-----\n";
var_dump(str_getcsv('.foo  . .  bar  .', ' ', '.', ''));
print "-----\n";
var_dump(str_getcsv('" "" "', ' '));
print "-----\n";
var_dump(str_getcsv(NULL));
print "-----\n";
var_dump(str_getcsv(''));
print "-----\n";

?>