<?php

class X {
  public $pub_var = null;
  public $pub_set = array();
  private $priv_var = 2;
  function __get($name) {
    echo 'get: ';
 var_dump($name);
 return $name == 'buz' ? 1 : array();
  }
  function __isset($name) {
    echo 'isset: ';
 var_dump($name);
    return $name == 'baz' || $name == 'buz';
  }
}
$x = new X;
var_dump(isset($x->foo));
var_dump(isset($x->baz));
var_dump(isset($x->buz));
var_dump(isset($x->pub_var));
var_dump(isset($x->pub_set));
var_dump(isset($x->priv_var));
var_dump(empty($x->foo));
var_dump(empty($x->baz));
var_dump(empty($x->buz));
var_dump(empty($x->pub_var));
var_dump(empty($x->pub_set));
var_dump(empty($x->priv_var));
unset($x->pub_var);
var_dump(isset($x->pub_var));
var_dump(empty($x->pub_var));
