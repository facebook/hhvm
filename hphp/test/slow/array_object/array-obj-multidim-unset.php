<?php

$arrayobj = new ArrayObject(array('foo'=>array('bar' => 2)));
unset($arrayObj['foo']['bar']);
var_dump(isset($arrayObj['foo']['bar']));
