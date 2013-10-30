<?php

$res = fopen('glob://' . __DIR__ . '/../sample_dir/*', 'r');
var_dump($res);
