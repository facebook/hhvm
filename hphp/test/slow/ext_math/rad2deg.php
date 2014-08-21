<?php

$rad = 1;
$deg = rad2deg($rad);
printf("rad: %.f = deg: %.20f \n", $rad, $deg);

$deg = 1;
$rad = deg2rad($deg);
printf("rad: %.20f = deg: %.f \n", $rad, $deg);
