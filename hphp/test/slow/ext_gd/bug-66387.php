<?php


<<__EntryPoint>>
function main_bug_66387() {
$im = imagecreatetruecolor(20, 20);
$c = imagecolorallocate($im, 255, 0, 0);
imagefilltoborder($im, 0, -999355, $c, $c);

echo "Done.\n";
}
