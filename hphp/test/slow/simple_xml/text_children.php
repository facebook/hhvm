<?php

$element = simplexml_load_string('<root><hello>world</hello></root>');
var_dump($element->hello->children());
var_dump((bool)$element->hello->children());
