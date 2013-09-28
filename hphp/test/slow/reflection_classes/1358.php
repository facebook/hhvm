<?php

class fOo {
}
interface ioO {
}
$c = new ReflectionClass('Foo');
$i = new ReflectionClass('Ioo');
var_dump($c->getFileName() !== '');
var_dump($i->getFileName() !== '');
