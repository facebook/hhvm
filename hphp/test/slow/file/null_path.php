<?hh


<<__EntryPoint>>
function main_null_path() {
error_reporting(-1);

$path1 = '/etc/passwd' . chr(0) . '/foo/bar.jpg';
$path2 = '/etc/shadow' . chr(0) . '/bar/baz.jpg';

var_dump(fopen($path1, 'r+'));
var_dump(popen($path1, 'r+'));
var_dump(file_get_contents($path1));
var_dump(file_put_contents($path1, 'data'));
var_dump(file($path1));
var_dump(readfile($path1));
var_dump(parse_ini_file($path1));
var_dump(md5_file($path1));
var_dump(sha1_file($path1));
var_dump(fileperms($path1));
var_dump(fileinode($path1));
var_dump(filesize($path1));
var_dump(fileowner($path1));
var_dump(filegroup($path1));
var_dump(fileatime($path1));
var_dump(filemtime($path1));
var_dump(filectime($path1));
var_dump(filetype($path1));
var_dump(linkinfo($path1));
var_dump(is_writable($path1));
var_dump(is_writeable($path1));
var_dump(is_readable($path1));
var_dump(is_executable($path1));
var_dump(is_file($path1));
var_dump(is_dir($path1));
var_dump(is_link($path1));
var_dump(file_exists($path1));
var_dump(stat($path1));
var_dump(lstat($path1));
var_dump(realpath($path1));
var_dump(disk_free_space($path1));
var_dump(diskfreespace($path1));
var_dump(disk_total_space($path1));
var_dump(chmod($path1, 644));
var_dump(chown($path1, 'nobody'));
var_dump(lchown($path1, 'nobody'));
var_dump(chgrp($path1, 'nogrp'));
var_dump(lchgrp($path1, 'nogrp'));
var_dump(touch($path1));
var_dump(copy($path1, $path2));
var_dump(rename($path1, $path2));
var_dump(unlink($path1, $path2));
var_dump(link($path1, $path2));
var_dump(symlink($path1, $path2));
var_dump(fnmatch($path1, $path2));
var_dump(tempnam($path1, 'tmp'));
var_dump(mkdir($path1));
var_dump(chdir($path1));
var_dump(chroot($path1));
var_dump(scandir($path1));
}
