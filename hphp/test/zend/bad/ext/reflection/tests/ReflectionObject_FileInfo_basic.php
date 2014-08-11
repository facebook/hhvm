<?php
$rc = new ReflectionObject(new C);
var_dump($rc->getFileName());
var_dump($rc->getStartLine());
var_dump($rc->getEndLine());

$rc = new ReflectionObject(new stdclass);
var_dump($rc->getFileName());
var_dump($rc->getStartLine());
var_dump($rc->getEndLine());

Class C {

}
?>
