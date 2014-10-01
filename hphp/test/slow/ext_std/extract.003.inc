<?php
$change = array('this' => 'bar');
var_dump(extract($change, EXTR_OVERWRITE));
var_dump($this);
