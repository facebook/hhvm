<?php

include_once 'base.php';

f('json_encode', String,  array('value' => Variant,
                                'loose' => array(Boolean, 'false')));
f('json_decode', Variant, array('json' => String,
                                'assoc' => array(Boolean, 'false'),
                                'loose' => array(Boolean, 'false')));
