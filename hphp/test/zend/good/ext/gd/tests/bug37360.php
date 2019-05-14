<?php <<__EntryPoint>> function main() {
$im = imagecreatefromgif(dirname(__FILE__) . '/bug37360.gif');
var_dump($im);
}
