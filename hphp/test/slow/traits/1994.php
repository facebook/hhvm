<?php

class this_is_a_class {
  function class_func() {
}
}
trait this_is_a_trait {
  function trait_func() {
}
}
interface this_is_an_interface {
  function interface_func();
}
$rclass = new ReflectionClass('this_is_a_class');
var_dump($rclass->isTrait());
$rtrait = new ReflectionClass('this_is_a_trait');
var_dump($rtrait->isTrait());
$rinterface = new ReflectionClass('this_is_an_interface');
var_dump($rinterface->isTrait());
