<?php

namespace foo;

class bar {
}

class_alias('foo\bar', 'foo\baz');

var_dump(new namespace\baz);

?>