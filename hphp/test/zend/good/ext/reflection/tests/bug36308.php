<?php
class Base {
    /** Base comment block */
    public $foo = 'bar';
}

class Extended extends Base {
    /** Extended commentary */
    public $foo = 'zim';
}

$reflect = new ReflectionClass('Extended');
$props = $reflect->getProperties();
echo $props[0]->getDocComment();
?>
