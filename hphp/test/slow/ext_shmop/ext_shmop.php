<?hh


// Most of the standard testing is in the zend shmop test.

// Check for key too large
<<__EntryPoint>>
function main_ext_shmop() :mixed{
$shm_id = shmop_open(0x1FFFFFFFF, "n", 0644, 1024);
invariant(!$shm_id, "");
}
