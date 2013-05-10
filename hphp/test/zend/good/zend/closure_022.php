<?php
$a = 0;
$foo = function() use ($a) {
};
$foo->a = 1;
?>