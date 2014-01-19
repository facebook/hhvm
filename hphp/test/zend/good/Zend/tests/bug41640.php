<?php
class foo {
        const FOO = 1;
        public $x = self::FOO;
}

var_dump(get_class_vars("foo"));