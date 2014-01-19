<?php

class Example {
    public $public = 'prop:public';
    private $prv   = 'prop:private';
    protected $prt = 'prop:protected';
}

$arrayobj = new ArrayObject(new Example());
var_dump($arrayobj->count());

$arrayobj = new ArrayObject(array('first','second','third'));
var_dump($arrayobj->count());
