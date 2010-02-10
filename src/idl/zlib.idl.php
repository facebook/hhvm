<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// zlib functions

f('readgzfile', Variant,
  array('filename' => String,
        'use_include_path' => array(Boolean, 'false')));

f('gzfile', Variant,
  array('filename' => String,
        'use_include_path' => array(Boolean, 'false')));

f('gzcompress', Variant,
  array('data' => String,
        'level' => array(Int32, '-1')));

f('gzuncompress', Variant,
  array('data' => String,
        'limit' => array(Int32, '0')));

f('gzdeflate', Variant,
  array('data' => String,
        'level' => array(Int32, '-1')));

f('gzinflate', Variant,
  array('data' => String,
        'limit' => array(Int32, '0')));

f('gzencode', Variant,
  array('data' => String,
        'level' => array(Int32, '-1'),
        'encoding_mode' => array(Int32, 'k_FORCE_GZIP')));

f('gzdecode', Variant,
  array('data' => String));

f('zlib_get_coding_type', String);

///////////////////////////////////////////////////////////////////////////////
// stream functions

f('gzopen', Resource,
  array('filename' => String,
        'mode' => String,
        'use_include_path' => array(Boolean, 'false')));

f('gzclose', Boolean,
  array('zp' => Resource));

f('gzrewind', Boolean,
  array('zp' => Resource));

f('gzeof', Boolean,
  array('zp' => Resource));

f('gzgetc', Variant,
  array('zp' => Resource));

f('gzgets', Variant,
  array('zp' => Resource,
        'length' => array(Int64, '1024')));

f('gzgetss', Variant,
  array('zp' => Resource,
        'length' => array(Int64, '0'),
        'allowable_tags' => array(String, 'null_string')));

f('gzread', Variant,
  array('zp' => Resource,
        'length' => array(Int64, '0')));

f('gzpassthru', Variant,
  array('zp' => Resource));

f('gzseek', Variant,
  array('zp' => Resource,
        'offset' => Int64,
        'whence' => array(Int64, 'SEEK_SET')));

f('gztell', Variant,
  array('zp' => Resource));

f('gzwrite', Variant,
  array('zp' => Resource,
        'str' => String,
        'length' => array(Int64, '0')));

f('gzputs', Variant,
  array('zp' => Resource,
        'str' => String,
        'length' => array(Int64, '0')));
