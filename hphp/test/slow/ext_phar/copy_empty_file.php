<?php
echo "*** Testing copy() function: file of 0 bytes size --\n";

var_dump( copy('phar://'.__DIR__.'/copy_empty_file.phar/empty_file', __DIR__.'/copy_empty_file.target') );
@unlink(__DIR__.'/copy_empty_file.target');
