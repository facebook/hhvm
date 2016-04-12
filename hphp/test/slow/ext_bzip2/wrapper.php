<?php

$loremIpsum = <<<LOREM_IPSUM
Lorem ipsum dolor sit amet,
consectetur adipiscing elit,
sed do eiusmod tempor incididunt
ut labore et dolore magna aliqua.

Ut enim ad minim veniam,
quis nostrud exercitation ullamco laboris
nisi ut aliquip ex ea commodo consequat.

Duis aute irure dolor in reprehenderit
in voluptate velit esse cillum dolore
eu fugiat nulla pariatur.

Excepteur sint occaecat cupidatat non proident,
sunt in culpa qui officia deserunt mollit anim id est laborum.

LOREM_IPSUM;

$fn = tempnam(sys_get_temp_dir(), "hhvm-bz2-");
$fp = file_put_contents("compress.bzip2://$fn", $loremIpsum);
echo "Written: ";
var_dump(file_exists($fn));
echo "Readable: ";
var_dump(is_readable($fn));

// bz2 should be able to compress it by at least 25%
$compressedSize = filesize($fn);
echo "Contains data: ";
var_dump($compressedSize > 0);
echo "Compressed from original: ";
var_dump($compressedSize < (strlen($loremIpsum) * 3/4));

$readContents = file_get_contents("compress.bzip2://$fn");
echo 'Match: ';
var_dump($loremIpsum === $readContents);

echo "----- Contents -----\n";
readfile("compress.bzip2://$fn");

unlink($fn);
