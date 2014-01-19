<?php
$f = fopen("php://fd/1", "rkkk");
fwrite($f, "hi!");

echo "\nDone.\n";