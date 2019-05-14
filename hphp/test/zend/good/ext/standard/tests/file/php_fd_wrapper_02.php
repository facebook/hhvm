<?php <<__EntryPoint>> function main() {
$f = fopen("php://fd/1", "rkkk");
fwrite($f, "hi!");

echo "\nDone.\n";
}
