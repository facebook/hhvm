<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('hphp_get_extension_info', VariantMap,
  array('name' => String));

f('hphp_get_class_info', VariantMap,
  array('name' => Variant));

f('hphp_get_function_info', VariantMap,
  array('name' => String));

f('hphp_invoke', Variant,
  array('name' => String,
        'params' => VariantVec));

f('hphp_invoke_method', Variant,
  array('obj' => Variant,
        'cls' => String,
        'name' => String,
        'params' => VariantVec));

f('hphp_instanceof', Boolean,
  array('obj' => Object,
        'name' => String));

f('hphp_create_object', Object,
  array('name' => String,
        'params' => VariantVec));

f('hphp_get_property', Variant,
  array('obj' => Object,
        'cls' => String,
        'prop' => String));

f('hphp_set_property', NULL,
  array('obj' => Object,
        'cls' => String,
        'prop' => String,
        'value' => Variant));

f('hphp_get_static_property', Variant,
  array('cls' => String,
        'prop' => String));

f('hphp_set_static_property', NULL,
  array('cls' => String,
        'prop' => String,
        'value' => Variant));
