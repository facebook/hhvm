<?php
class foo
{
	private $private = 'private';
	protected $protected = 'protected';
	public $public = 'public';

    public function __sleep()
    {
        return array('private', 'protected', 'public');
    }
}

var_dump(str_replace("\0", '\0', serialize(new foo())));
?>