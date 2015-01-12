<?php

$a = ['img99.jpg', 'img1.jpg', 'img2.jpg', 'img12.jpg', 'img10.jpg'];
array_multisort($a);
var_dump($a);
array_multisort($a, SORT_ASC, SORT_NATURAL);
var_dump($a);
array_multisort($a, SORT_DESC, SORT_NATURAL);
var_dump($a);
