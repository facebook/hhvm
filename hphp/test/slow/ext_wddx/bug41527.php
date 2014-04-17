<?php
$data = array('01' => 'Zero', '+1' => 'Plus sign', ' 1' => 'Space');

var_dump(wddx_deserialize(wddx_serialize_vars('data')));
?>
