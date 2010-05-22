<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// file handle based file operations

f('fopen', Variant,
  array('filename' => String,
        'mode' => String,
        'use_include_path' => array(Boolean, 'false'),
        'context' => array(Resource, 'null_object')));

f('popen', Variant,
  array('command' => String,
        'mode' => String));

f('fclose', Boolean,
  array('handle' => Resource));

f('pclose', Variant,
  array('handle' => Resource));

f('fseek', Variant,
  array('handle' => Resource,
        'offset' => Int64,
        'whence' => array(Int64, 'SEEK_SET')));

f('rewind', Boolean,
  array('handle' => Resource));

f('ftell', Variant,
  array('handle' => Resource));

f('feof', Boolean,
  array('handle' => Resource));

f('fstat', Variant,
  array('handle' => Resource));

f('fread', Variant,
  array('handle' => Resource,
        'length' => Int64));

f('fgetc', Variant,
  array('handle' => Resource));

f('fgets', Variant,
  array('handle' => Resource,
        'length' => array(Int64, '1024')));

f('fgetss', Variant,
  array('handle' => Resource,
        'length' => array(Int64, '0'),
        'allowable_tags' => array(String, 'null_string')));

f('fscanf', Variant,
  array('handle' => Resource,
        'format' => String),
  ReferenceVariableArguments);

f('fpassthru', Variant,
  array('handle' => Resource));

f('fwrite', Variant,
  array('handle' => Resource,
        'data' => String,
        'length' => array(Int64, '0')));

f('fputs', Variant,
  array('handle' => Resource,
        'data' => String,
        'length' => array(Int64, '0')));

f('fprintf', Variant,
  array('handle' => Resource,
        'format' => String),
  VariableArguments);

f('vfprintf', Variant,
  array('handle' => Resource,
        'format' => String,
        'args' => VariantVec));

f('fflush', Boolean,
  array('handle' => Resource));

f('ftruncate', Boolean,
  array('handle' => Resource,
        'size' => Int64));

f('flock', Boolean,
  array('handle' => Resource,
        'operation' => Int32,
        'wouldblock' => array(Boolean | Reference, 'null')));

f('fputcsv', Variant,
  array('handle' => Resource,
        'fields' => VariantVec,
        'delimiter' => array(String, '","'),
        'enclosure' => array(String, '"\""')));

f('fgetcsv', Variant,
  array('handle' => Resource,
        'length' => array(Int64, '0'),
        'delimiter' => array(String, '","'),
        'enclosure' => array(String, '"\""')));

///////////////////////////////////////////////////////////////////////////////
// file name based file operations

f('file_get_contents', Variant,
  array('filename' => String,
        'use_include_path' => array(Boolean, 'false'),
        'context' => array(Resource, 'null_object'),
        'offset' => array(Int64, '0'),
        'maxlen' => array(Int64, '0')));

f('file_put_contents', Variant,
  array('filename' => String,
        'data' => Variant,
        'flags' => array(Int32, '0'),
        'context' => array(Resource, 'null_object')));

f('file', Variant,
  array('filename' => String,
        'flags' => array(Int32, '0'),
        'context' => array(Resource, 'null_object')));

f('readfile', Variant,
  array('filename' => String,
        'use_include_path' => array(Boolean, 'false'),
        'context' => array(Resource, 'null_object')));

f('move_uploaded_file', Boolean,
  array('filename' => String,
        'destination' => String));

f('parse_ini_file', Variant,
  array('filename' => String,
        'process_sections' => array(Boolean, 'false'),
        'scanner_mode' => array(Int32, 'k_INI_SCANNER_NORMAL')));

f('parse_ini_string', Variant,
  array('ini' => String,
        'process_sections' => array(Boolean, 'false'),
        'scanner_mode' => array(Int32, 'k_INI_SCANNER_NORMAL')));

f('parse_hdf_file', Variant,
  array('filename' => String));

f('parse_hdf_string', Variant,
  array('input' => String));

f('write_hdf_file', Boolean,
  array('data' => VariantMap,
        'filename' => String));

f('write_hdf_string', String,
  array('data' => VariantMap));

f('md5_file', Variant,
  array('filename' => String,
        'raw_output' => array(Boolean, 'false')));

f('sha1_file', Variant,
  array('filename' => String,
        'raw_output' => array(Boolean, 'false')));

///////////////////////////////////////////////////////////////////////////////
// shell commands

f('chmod', Boolean,
  array('filename' => String,
        'mode' => Int64));

f('chown', Boolean,
  array('filename' => String,
        'user' => Primitive));

f('lchown', Boolean,
  array('filename' => String,
        'user' => Variant));

f('chgrp', Boolean,
  array('filename' => String,
        'group' => Primitive));

f('lchgrp', Boolean,
  array('filename' => String,
        'group' => Variant));

f('touch', Boolean,
  array('filename' => String,
        'mtime' => array(Int64, '0'),
        'atime' => array(Int64, '0')));

f('copy', Boolean,
  array('source' => String,
        'dest' => String,
        'context' => array(Resource, 'null_object')));

f('rename', Boolean,
  array('oldname' => String,
        'newname' => String,
        'context' => array(Resource, 'null_object')));

f('umask', Int32,
  array('mask' => array(Variant, 'null_variant')));

f('unlink', Boolean,
  array('filename' => String,
        'context' => array(Resource, 'null_object')));

f('link', Boolean,
  array('target' => String,
        'link' => String));

f('symlink', Boolean,
  array('target' => String,
        'link' => String));

f('basename', String,
  array('path' => String,
        'suffix' => array(String, 'null_string')));

f('fnmatch', Boolean,
  array('pattern' => String,
        'filename' => String,
        'flags' => array(Int32, '0')));

f('glob', Variant,
  array('pattern' => String,
        'flags' => array(Int32, '0')));

f('tempnam', Variant,
  array('dir' => String,
        'prefix' => String));

f('tmpfile', Variant);

///////////////////////////////////////////////////////////////////////////////
// stats functions

f('fileperms',        Variant, array('filename' => String));
f('fileinode',        Variant, array('filename' => String));
f('filesize',         Variant, array('filename' => String));
f('fileowner',        Variant, array('filename' => String));
f('filegroup',        Variant, array('filename' => String));
f('fileatime',        Variant, array('filename' => String));
f('filemtime',        Variant, array('filename' => String));
f('filectime',        Variant, array('filename' => String));
f('filetype',         Variant, array('filename' => String));
f('linkinfo',         Variant, array('filename' => String));
f('is_writable',      Boolean, array('filename' => String));
f('is_writeable',     Boolean, array('filename' => String));
f('is_readable',      Boolean, array('filename' => String));
f('is_executable',    Boolean, array('filename' => String));
f('is_file',          Boolean, array('filename' => String));
f('is_dir',           Boolean, array('filename' => String));
f('is_link',          Boolean, array('filename' => String));
f('is_uploaded_file', Boolean, array('filename' => String));
f('file_exists',      Boolean, array('filename' => String));
f('stat',             Variant, array('filename' => String));
f('lstat',            Variant, array('filename' => String));
f('clearstatcache');
f('readlink',         Variant, array('path' => String));
f('realpath',         Variant, array('path' => String));
f('pathinfo',         Variant, array('path' => String,
                                     'opt' => array(Int32, '15')));
f('disk_free_space',  Variant, array('directory' => String));
f('diskfreespace',    Variant, array('directory' => String));
f('disk_total_space', Variant, array('directory' => String));

///////////////////////////////////////////////////////////////////////////////
// directory functions

f('mkdir', Boolean,
  array('pathname' => String,
        'mode' => array(Int64, '0777'),
        'recursive' => array(Boolean, 'false'),
        'context' => array(Resource, 'null_object')));

f('rmdir', Boolean,
  array('dirname' => String,
        'context' => array(Resource, 'null_object')));

f('dirname', String,
  array('path' => String));

f('getcwd', Variant);

f('chdir', Boolean,
  array('directory' => String));

f('chroot', Boolean,
  array('directory' => String));

f('dir', Variant,
  array('directory' => String));

f('opendir', Variant,
  array('path' => String,
        'context' => array(Resource, 'null')));

f('readdir', Variant,
  array('dir_handle' => Resource));

f('rewinddir', NULL,
  array('dir_handle' => Resource));

f('scandir', Variant,
  array('directory' => String,
        'descending' => array(Boolean, 'false'),
        'context' => array(Resource, 'null')));

f('closedir', NULL,
  array('dir_handle' => Resource));
