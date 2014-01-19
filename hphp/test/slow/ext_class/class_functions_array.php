<?php

var_dump(method_exists(array("foo" => 42), "foo"));
var_dump(get_class_methods(array("bar")));
var_dump(is_a(array("bar"), "foo"));
var_dump(is_subclass_of(array("bar"), "foo"));
