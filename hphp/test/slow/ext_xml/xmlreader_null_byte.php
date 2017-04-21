<?php

$file = '/etc/passwd'.chr(0).'asdf';

$reader = new XMLReader();
var_dump($reader->open($file));
var_dump($reader->setRelaxNGSchema($file));
