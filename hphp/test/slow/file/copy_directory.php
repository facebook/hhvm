<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_copy_directory() {

$src_dir = sys_get_temp_dir().'/'.'copy_dir_src';
$dest_dir = sys_get_temp_dir().'/'.'copy_dir_dest';

// Create the source directory
mkdir("$src_dir");
$fh = fopen("$src_dir/a.txt", 'w');
fwrite($fh, "a\n");
fclose($fh);
$fh = fopen("$src_dir/b.txt", 'w');
fwrite($fh, "b\n");
fclose($fh);

// Try to copy it.  This should fail.
copy("$src_dir", "$dest_dir");
// The destination path should not exist now.
echo "dest exists: " . (file_exists($dest_dir) ? "true" : "false");

unlink("$src_dir/a.txt");
unlink("$src_dir/b.txt");
rmdir("$src_dir");
}
