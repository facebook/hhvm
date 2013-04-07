<?php
error_reporting(E_ALL);

trait foo
{
    public $zoo = 'foo::zoo';
}

class baz
{
    use foo;
    public $zoo = 'baz::zoo';
}

$obj = new baz();
echo $obj->zoo, "\n";
?>