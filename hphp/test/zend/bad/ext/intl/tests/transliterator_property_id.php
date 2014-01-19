<?php
$tr = Transliterator::create("Katakana-Latin");
echo $tr->id, "\n";
$revtr = $tr->createInverse();
echo $revtr->id, "\n";
var_dump($revtr);

echo "Done.\n";