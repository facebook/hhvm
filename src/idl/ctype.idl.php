<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// character type functions

f('ctype_alnum',  Boolean, array('text' => Variant));
f('ctype_alpha',  Boolean, array('text' => Variant));
f('ctype_cntrl',  Boolean, array('text' => Variant));
f('ctype_digit',  Boolean, array('text' => Variant));
f('ctype_graph',  Boolean, array('text' => Variant));
f('ctype_lower',  Boolean, array('text' => Variant));
f('ctype_print',  Boolean, array('text' => Variant));
f('ctype_punct',  Boolean, array('text' => Variant));
f('ctype_space',  Boolean, array('text' => Variant));
f('ctype_upper',  Boolean, array('text' => Variant));
f('ctype_xdigit', Boolean, array('text' => Variant));
