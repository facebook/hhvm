<?php

var_dump(get_resource_type());
var_dump(get_resource_type(""));
$fp = fopen(__FILE__, "r");
var_dump(get_resource_type($fp));
fclose($fp);
var_dump(get_resource_type($fp));

var_dump(gettype(get_loaded_extensions()));
var_dump(count(get_loaded_extensions()));
var_dump(gettype(get_loaded_extensions(true)));
var_dump(count(get_loaded_extensions(true)));
var_dump(get_loaded_extensions(true, true));

define("USER_CONSTANT", "test");

var_dump(get_defined_constants(true, true));
var_dump(gettype(get_defined_constants(true)));
var_dump(gettype(get_defined_constants()));
var_dump(count(get_defined_constants()));

function test () {
}

var_dump(get_defined_functions(true));
var_dump(gettype(get_defined_functions()));
var_dump(count(get_defined_functions()));

var_dump(get_declared_interfaces(true));
var_dump(gettype(get_declared_interfaces()));
var_dump(count(get_declared_interfaces()));

var_dump(get_extension_funcs());
var_dump(get_extension_funcs(true));
var_dump(gettype(get_extension_funcs("standard")));
var_dump(count(get_extension_funcs("standard")));
var_dump(gettype(get_extension_funcs("zend")));
var_dump(count(get_extension_funcs("zend")));


echo "Done\n";
?>