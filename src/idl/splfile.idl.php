<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// splfileinfo

f('hphp_splfileinfo___construct', Resource,
  array('obj' => Resource, 'file_name' => String));

f('hphp_splfileinfo_getatime', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getbasename', String,
  array('obj' => Resource, 'suffix' => String));

f('hphp_splfileinfo_getctime', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getfileinfo', Resource,
  array('obj' => Resource, 'class_name' => String));

f('hphp_splfileinfo_getfilename', String,
  array('obj' => Resource));

f('hphp_splfileinfo_getgroup', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getinode', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getlinktarget', String,
  array('obj' => Resource));

f('hphp_splfileinfo_getmtime', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getowner', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getpath', String,
  array('obj' => Resource));

f('hphp_splfileinfo_getpathinfo', Resource,
  array('obj' => Resource, 'class_name' => String));

f('hphp_splfileinfo_getpathname', String,
  array('obj' => Resource));

f('hphp_splfileinfo_getperms', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_getrealpath', Variant,
  array('obj' => Resource));

f('hphp_splfileinfo_getsize', Int64,
  array('obj' => Resource));

f('hphp_splfileinfo_gettype', String,
  array('obj' => Resource));

f('hphp_splfileinfo_isdir', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_isexecutable', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_isfile', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_islink', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_isreadable', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_iswritable', Boolean,
  array('obj' => Resource));

f('hphp_splfileinfo_openfile', Resource,
  array('obj' => Resource, 'open_mode' => String,
        'use_include_path' => Boolean, 'context' => Variant));

f('hphp_splfileinfo_setfileclass', NULL,
  array('obj' => Resource, 'class_name' => String));

f('hphp_splfileinfo_setinfoclass', NULL,
  array('obj' => Resource, 'class_name' => String));

f('hphp_splfileinfo___tostring', String,
  array('obj' => Resource));

///////////////////////////////////////////////////////////////////////////////
// splfileobject
f('hphp_splfileobject___construct', Resource,
  array('obj' => Resource, 'filename' => String, 'open_mode' => String,
        'use_include_path' => Boolean, 'context' => Variant));

f('hphp_splfileobject_current', Variant,
  array('obj' => Resource));

f('hphp_splfileobject_eof', Boolean,
  array('obj' => Resource));

f('hphp_splfileobject_fflush', Boolean,
  array('obj' => Resource));

f('hphp_splfileobject_fgetc', String,
  array('obj' => Resource));

f('hphp_splfileobject_fgetcsv', Variant,
  array('obj' => Resource, 'delimiter' => String, 'enclosure' => String,
        'escape' => String));

f('hphp_splfileobject_fgets', String,
  array('obj' => Resource));

f('hphp_splfileobject_fgetss', String,
  array('obj' => Resource, 'allowable_tags' => String));

f('hphp_splfileobject_flock', Boolean,
  array('obj' => Resource, 'wouldblock' => Variant | Reference));

f('hphp_splfileobject_fpassthru', Int64,
  array('obj' => Resource));

f('hphp_splfileobject_fscanf', Variant,
  array('_argc' => Int64, 'obj' => Resource, 'format' => String,
        '_argv' => Variant));

f('hphp_splfileobject_fseek', Int64,
  array('obj' => Resource, 'offset' => Int64, 'whence' => Int64));

f('hphp_splfileobject_fstat', Variant,
  array('obj' => Resource));

f('hphp_splfileobject_ftell', Int64,
  array('obj' => Resource));

f('hphp_splfileobject_ftruncate', Boolean,
  array('obj' => Resource, 'size' => Int64));

f('hphp_splfileobject_fwrite', Int64,
  array('obj' => Resource, 'str' => String, 'length' => Int64));

f('hphp_splfileobject_getcvscontrol', Variant,
  array('obj' => Resource));

f('hphp_splfileobject_getflags', Int64,
  array('obj' => Resource));

f('hphp_splfileobject_getmaxlinelen', Int64,
  array('obj' => Resource));

f('hphp_splfileobject_key', Int64,
  array('obj' => Resource));

f('hphp_splfileobject_next', NULL,
  array('obj' => Resource));

f('hphp_splfileobject_rewind', NULL,
  array('obj' => Resource));

f('hphp_splfileobject_valid', Boolean,
  array('obj' => Resource));

f('hphp_splfileobject_seek', NULL,
  array('obj' => Resource, 'line_pos' => Int64));

f('hphp_splfileobject_setcsvcontrol', NULL,
  array('obj' => Resource, 'delimiter' => String,
        'enclosure' => String, 'escape' => String));

f('hphp_splfileobject_setflags', NULL,
  array('obj' => Resource, 'flags' => Int64));

f('hphp_splfileobject_setmaxlinelen', NULL,
  array('obj' => Resource, 'max_len' => Int64));
