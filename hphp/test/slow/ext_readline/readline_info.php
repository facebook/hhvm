<?php
error_reporting(E_ALL & ~E_WARNING);

// Changeable
readline_info('readline_name', 123); // This gets converted to string

var_dump(readline_info('line_buffer', 'Buffer Content') !=
         readline_info('line_buffer'));

var_dump(readline_info('readline_name'));

// Not Changeable
var_dump(readline_info('library_version', 'Fake') ==
         readline_info('library_version'));
