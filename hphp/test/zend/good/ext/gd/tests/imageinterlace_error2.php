<?php <<__EntryPoint>> function main() {
$image = fopen('php://stdin', 'r');
var_dump(imageinterlace($image));
}
