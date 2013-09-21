<?php
$h = new SplMaxHeap();

$h->insert(1);
$h->insert(5);
$h->insert(0);
$h->insert(4);

$h->rewind();
echo "count(\$h) = ".count($h)."\n";
echo "\$h->count() = ".$h->count()."\n";

while ($h->valid()) {
    $k = $h->key();
    $v = $h->current();
    echo "$k=>$v\n";
    $h->next();
}
?>
===DONE===
<?php exit(0); ?>