<?php

namespace foo;

class bar { }

class_alias('foo\bar', 'baz');

use \baz as stdClass;

var_dump(new bar);
var_dump(new stdClass);
var_dump(new \baz);

?>