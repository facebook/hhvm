<?php
class foo
{
	private $private = 'private';
	protected $protected = 'protected';
	public $public = 'public';

	public function __sleep()
	{
		return array('private', 'protected', 'public', 'no_such');
	}
}
$foo = new foo();
$data = serialize($foo);
var_dump(str_replace("\0", '\0', $data));
?>