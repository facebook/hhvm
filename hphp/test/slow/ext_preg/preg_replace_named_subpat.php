<?php

$content = 'b';
$pattern = '/(?P<a>.*)/';

preg_replace_callback($pattern, function ($m) { var_dump($m); }, $content);
