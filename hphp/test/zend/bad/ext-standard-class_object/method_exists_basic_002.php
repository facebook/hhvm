<?php
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo " ---(Internal classes, using string class name)---\n";
echo "Does exception::getmessage exist? ";
var_dump(method_exists("exception", "getmessage"));
echo "Does stdclass::nonexistent exist? ";
var_dump(method_exists("stdclass", "nonexistent"));

echo "\n ---(Internal classes, using class instance)---\n";
echo "Does exception::getmessage exist? ";
var_dump(method_exists(new exception, "getmessage"));
echo "Does stdclass::nonexistent exist? ";
var_dump(method_exists(new stdclass, "nonexistent"));

echo "Done";
?>