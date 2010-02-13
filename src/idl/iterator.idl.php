<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// recursiveiteratoriterator

f('hphp_recursiveiteratoriterator___construct', Resource,
  array('obj' => Resource, 'iterator' => Resource,
        'mode' => Int64, 'flags' => Int64));

f('hphp_recursiveiteratoriterator_getinneriterator', Resource,
  array('obj' => Resource));

f('hphp_recursiveiteratoriterator_current', Variant,
  array('obj' => Resource));

f('hphp_recursiveiteratoriterator_key', Variant,
  array('obj' => Resource));

f('hphp_recursiveiteratoriterator_next', NULL,
  array('obj' => Resource));

f('hphp_recursiveiteratoriterator_rewind', NULL,
  array('obj' => Resource));

f('hphp_recursiveiteratoriterator_valid', Boolean,
  array('obj' => Resource));

///////////////////////////////////////////////////////////////////////////////
// directoryiterator

f('hphp_directoryiterator___construct', Resource,
  array('obj' => Resource, 'path' => String));

f('hphp_directoryiterator_key', Variant,
  array('obj' => Resource));

f('hphp_directoryiterator_next', NULL,
  array('obj' => Resource));

f('hphp_directoryiterator_rewind', NULL,
  array('obj' => Resource));

f('hphp_directoryiterator_seek', NULL,
  array('obj' => Resource, 'position' => Int64));

f('hphp_directoryiterator_current', Variant,
  array('obj' => Resource));

f('hphp_directoryiterator___tostring', String,
  array('obj' => Resource));

f('hphp_directoryiterator_valid', Boolean,
  array('obj' => Resource));

f('hphp_directoryiterator_isdot', Boolean,
  array('obj' => Resource));

///////////////////////////////////////////////////////////////////////////////
// recursivedirectoryiterator

f('hphp_recursivedirectoryiterator___construct', Resource,
  array('obj' => Resource, 'path' => String, 'flags' => Int64));

f('hphp_recursivedirectoryiterator_key', Variant,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_next', NULL,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_rewind', NULL,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_seek', NULL,
  array('obj' => Resource, 'position' => Int64));

f('hphp_recursivedirectoryiterator_current', Variant,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator___tostring', String,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_valid', Boolean,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_haschildren', Boolean,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_getchildren', Resource,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_getsubpath', String,
  array('obj' => Resource));

f('hphp_recursivedirectoryiterator_getsubpathname', String,
  array('obj' => Resource));
