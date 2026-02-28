<?hh <<__EntryPoint>> function main(): void {
$flags = imagetypes();

if ($flags&0x1 && !function_exists("imagegif")) {
    echo "gif failed\n";
}

if ($flags&0x2 && !function_exists("imagejpeg")) {
    echo "jpeg failed\n";
}

if ($flags&0x4 && !function_exists("imagepng")) {
    echo "png failed\n";
}

if ($flags&0x8 && !function_exists("imagewbmp")) {
    echo "wbmp failed\n";
}

echo "ok\n";
}
