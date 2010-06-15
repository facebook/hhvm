<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('spl_classes', StringMap);

f('spl_object_hash', String,
  array('obj' => Object));

f('class_implements', Variant,
  array('obj' => Variant,
        'autoload' => array(Boolean, 'true')));

f('class_parents', Variant,
  array('obj' => Variant,
        'autoload' => array(Boolean, 'true')));

f('iterator_apply', Variant,
  array('obj' => Variant,
        'func' => Variant,
        'params' => array(VariantMap, 'null_array')));

f('iterator_count', Variant,
  array('obj' => Variant));

f('iterator_to_array', Variant,
  array('obj' => Variant,
        'use_keys' => array(Boolean, 'true')));

//f('spl_autoload_call');
//f('spl_autoload_extensions');
//f('spl_autoload_functions');
//f('spl_autoload_register');
//f('spl_autoload_unregister');
//f('spl_autoload');
