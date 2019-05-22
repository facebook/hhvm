<?php
class foo
{
    private $private = 'private';
    protected $protected = 'protected';
    public $public = 'public';
}
<<__EntryPoint>> function main() {
var_export((array) new foo);
}
