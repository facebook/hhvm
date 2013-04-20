<?php

$element = simplexml_load_string('<root><!-- I am a comment --></root>');
var_dump($element->count());

