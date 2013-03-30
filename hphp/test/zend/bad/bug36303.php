<?php
$x="test";
foreach($x->a->b as &$v) {
}
echo "ok\n";
?>