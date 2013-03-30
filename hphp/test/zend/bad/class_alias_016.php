<?php

namespace foo;

class bar {
}

class_alias('foo\bar', 'foo');

var_dump(new \foo);
var_dump(new foo);

?>