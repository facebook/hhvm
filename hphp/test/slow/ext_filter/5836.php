<?php
var_dump( filter_var_array(
  array(
    'zero' => '0',
    'false' => 'false',
    'no' => 'no',
    'off' => 'off',
    'empty' => '',
    'null' => null,
    'false-bool' => false,
  ),
  array(
    'false' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'zero' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'false' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'no' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'off' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'empty' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'null' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
    'false-bool' => array( 'filter' => FILTER_VALIDATE_BOOLEAN ),
  )
) );
