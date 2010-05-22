<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('get_declared_classes', VariantMap);
f('get_declared_interfaces', VariantMap);

f('class_exists', Boolean,
  array('class_name' => String,
        'autoload' => array(Boolean, 'true')));

f('interface_exists', Boolean,
  array('interface_name' => String,
        'autoload' => array(Boolean, 'true')));

f('get_class_methods', VariantMap,
  array('class_or_object' => Variant));

f('get_class_vars', VariantMap,
  array('class_name' => String));

///////////////////////////////////////////////////////////////////////////////

f('get_class', Variant,
  array('object' => array(Variant, 'null_variant')));

f('get_parent_class', Variant,
  array('object' => array(Variant, 'null_variant')));

f('is_a', Boolean,
  array('object' => Object,
        'class_name' => String));

f('is_subclass_of', Boolean,
  array('class_or_object' => Variant,
        'class_name' => String));

f('method_exists', Boolean,
  array('class_or_object' => Variant,
        'method_name' => String));

f('property_exists', Boolean,
  array('class_or_object' => Variant,
        'property' => String));

f('get_object_vars', VariantMap,
  array('object' => Object));

///////////////////////////////////////////////////////////////////////////////

f('call_user_method_array', Variant,
  array('method_name' => String,
        'obj' => Object | Reference,
        'paramarr' => VariantVec));

f('call_user_method', Variant,
  array('method_name' => String,
        'obj' => Object | Reference),
  VariableArguments);
