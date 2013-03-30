<?php
$i = 1;
$lambda = function () use ($i) {
    return ++$i;
};
$lambda();
echo $lambda()."\n";
//early prototypes gave 3 here because $i was static in $lambda
?>