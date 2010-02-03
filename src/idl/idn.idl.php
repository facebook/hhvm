<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('idn_to_ascii', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));

f('idn_to_unicode', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));

f('idn_to_utf8', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));
