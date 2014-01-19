<?php

$res = file_get_contents('glob://' . __DIR__ . '/../test/sample_dir/*');
var_dump($res);
