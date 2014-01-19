<?php

namespace foo;

const foo = 1;

define('foo', 2);

var_dump(foo, namespace\foo, \foo\foo, \foo, constant('foo'), constant('foo\foo'));

?>