<?php

$a = array(
  'test' => array(
    'sub_test' => array(

    ),
  ),
);

mb_convert_variables( 'utf-8', 'windows-1251', $a );
var_dump($a);
