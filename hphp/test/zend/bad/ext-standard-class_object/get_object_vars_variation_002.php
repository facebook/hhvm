<?php
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

$obj = new stdClass;
var_dump(get_object_vars($obj));

$a='original.a';
$obj->ref = &$a;
$obj->val = $a;

$arr = get_object_vars($obj);
var_dump($arr);

$arr['ref'] = 'changed.ref';
$arr['val'] = 'changed.val';

var_dump($arr, $obj, $a);
?>