<?php

$file = new SplFileObject(__FILE__);
var_dump($file->fread(5));
