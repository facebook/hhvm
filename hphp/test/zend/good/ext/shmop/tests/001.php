<?hh <<__EntryPoint>> function main(): void {
$hex_shm_id = 0xff3;
$write_d1 = "test #1 of the shmop() extension";
$write_d2 = "test #2 append data to shared memory segment";

echo "shm open for create: ";
$shm_id = shmop_open($hex_shm_id, "n", 0644, 1024);
if (!$shm_id) {
  exit("failed\n");
} else {
  echo "ok\n";
}

echo "shm size is: " . ($shm_size = shmop_size($shm_id)) . "\n";

echo "shm write test #1: ";
$written = shmop_write($shm_id, $write_d1, 0);
if ($written != strlen($write_d1)) {
  echo "failed\n";
} else {
  echo "ok\n";
}

echo "data in memory is: " . shmop_read($shm_id, 0, $written) . "\n";

shmop_close($shm_id);

echo "shm open for read only: ";
$shm_id = shmop_open($hex_shm_id, "a", 0644, 1024);
if (!$shm_id) {
  echo "failed\n";
} else {
  echo "ok\n";
}

echo "data in memory is: " . shmop_read($shm_id, 0, $written) . "\n";

/* try to append data to the shared memory segment, this should fail */
@shmop_write($shm_id, $write_d1, $written);

shmop_close($shm_id);

echo "shm open for read only: ";
$shm_id = shmop_open($hex_shm_id, "w", 0644, 1024);
if (!$shm_id) {
  echo "failed\n";
} else {
  echo "ok\n";
}

echo "shm write test #1: ";
$written = shmop_write($shm_id, $write_d2, $written);
if ($written != strlen($write_d2)) {
  exit("failed\n");
} else {
  echo "ok\n";
}

echo "data in memory is: " . shmop_read($shm_id, 0, strlen($write_d1 . $write_d2)) . "\n";

echo "deletion of shm segment: ";
if (!shmop_delete($shm_id)) {
  echo "failed\n";
} else {
  echo "ok\n";
}

shmop_close($shm_id);
}
