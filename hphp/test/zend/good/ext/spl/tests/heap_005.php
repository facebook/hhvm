<?php
$input = range(1,100);
shuffle(&$input);

$h = new SplMinHeap();

foreach($input as $i) {
    $h->insert($i);
}

foreach ($h as $k => $o) {
    echo "$k => $o\n";
}
echo "===DONE===\n";
