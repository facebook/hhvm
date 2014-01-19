<?php

interface a {
 }

abstract class b {
 }

final class c {
 }

trait d {
}

var_dump(class_exists('a'));
var_dump(class_exists('b'));
var_dump(class_exists('c'));
var_dump(class_exists('d'));

?>
