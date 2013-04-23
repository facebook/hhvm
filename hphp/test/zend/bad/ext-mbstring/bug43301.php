<?php

$ptr = 'hello';

$txt = <<<doc
hello, I have got a cr*sh on you
doc;

echo mb_ereg_replace($ptr,'$1',$txt,'e');

?>