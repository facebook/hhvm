<?php

trait foo { }

$rc = new ReflectionClass('foo');
$rc->newInstance();
