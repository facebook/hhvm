<?php
class foo
{
	private $private = 'private';
	protected $protected = 'protected';
	public $public = 'public';
}
var_export((array) new foo);
?>