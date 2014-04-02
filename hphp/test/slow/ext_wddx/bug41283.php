<?php
$data = array(
  'somearray' => array('1.1' => 'One 1','1.2' => 'One 2', '1.0' => 'Three')
);

var_dump(wddx_deserialize(wddx_serialize_vars('data')));
?>
