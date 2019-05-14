<?php <<__EntryPoint>> function main() {
$image = tmpfile();
$gamma = imagegammacorrect($image, 1.0, 5.0);
}
