<?php
class just_constants
{
    const BOOLEAN_CONSTANT = true;
    const NULL_CONSTANT = null;
    const STRING_CONSTANT = 'This is a string';
    const INTEGER_CONSTANT = 1000;
    const FLOAT_CONSTANT = 3.14159265;
}

Reflection::export(new ReflectionClass('just_constants'));
?>
