<?php
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized) 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

$serialized = 'O:1:"C":1:{s:1:"p";i:1;}';

$incomplete = unserialize($serialized);
eval('Class C {}');
$complete   = unserialize($serialized);


echo "\n\n---> Various types of access on complete class:\n" ;
var_dump($complete);
var_dump(is_object($complete));
var_dump($complete->p);

$ref1 = "ref1.original";
$complete->p = &$ref1;
var_dump($complete->p);
$ref1 = "ref1.changed";
var_dump($complete->p);
$complete->p = "p.changed";
var_dump($ref1);

var_dump(isset($complete->x));
$complete->x = "x.new";
var_dump(isset($complete->x));
unset($complete->x);
var_dump($complete->x);


echo "\n\n---> Same types of access on incomplete class:\n" ;
var_dump($incomplete);
var_dump(is_object($incomplete));
var_dump($incomplete->p);

$ref2 = "ref1.original";
$incomplete->p = &$ref2;
var_dump($incomplete->p);
$ref2 = "ref1.changed";
var_dump($incomplete->p);
$incomplete->p = "p.changed";
var_dump($ref1);

var_dump(isset($incomplete->x));
$incomplete->x = "x.new";
var_dump(isset($incomplete->x));
unset($incomplete->x);
var_dump($incomplete->x);

$incomplete->f();

echo "Done";
?>