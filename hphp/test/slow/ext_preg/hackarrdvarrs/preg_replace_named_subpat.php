<?php


<<__EntryPoint>>
function main_preg_replace_named_subpat() {
$content = 'b';
$pattern = '/(?P<a>.*)/';

preg_replace_callback($pattern, function ($m) { var_dump($m); }, $content);
}
