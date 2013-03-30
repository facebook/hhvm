<?php
class foo {
	private $private = 'private';
	protected $protected = 'protected';
	public $public = 'public';
}
$data = new foo();
$obj_vars = get_object_vars($data);
var_dump($obj_vars);
?>