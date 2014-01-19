<?php

trait T {
 public function y() {
}
 }
interface I {
 public function y();
}
class C {
 use T ;
 }
var_dump(method_exists('T', 'y'));
var_dump(method_exists('C', 'y'));
var_dump(method_exists('I', 'y'));
var_dump(method_exists('T', 'x'));
var_dump(method_exists('C', 'x'));
var_dump(method_exists('I', 'x'));
