<?hh
function flock_or_die($filename, $resource, $flock_op) {
  $wouldblock = false;
  $r = flock($resource, $flock_op, inout $wouldblock);
  var_dump($r);
 }


<<__EntryPoint>>
function main_1689() {
$filename = sys_get_temp_dir().'/flock_file.dat';
$resource = fopen($filename, 'w');
flock_or_die($filename, $resource, LOCK_EX);
flock_or_die($filename, $resource, LOCK_UN);
unlink($filename);
}
