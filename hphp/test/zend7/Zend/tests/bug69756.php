<?php
$arr = range(1, 2);
foreach($arr as &$item ) {
    var_dump($arr === array(1, 2));
}
?>
