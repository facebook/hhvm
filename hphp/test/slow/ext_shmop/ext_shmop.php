<?hh

// Most of the standard testing is in the zend shmop test.

// Check for key too large
$shm_id = shmop_open(0x1FFFFFFFF, "n", 0644, 1024);
assert(!$shm_id);
