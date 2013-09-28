<?php

var_dump( posix_access() );
var_dump( posix_access(array()) );
var_dump( posix_access('foo',array()) );
var_dump( posix_access(null) );

var_dump(posix_access('./foobar'));
?>
===DONE===