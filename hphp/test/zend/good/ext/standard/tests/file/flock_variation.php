<?hh
/*
Prototype: bool flock(resource $handle, int $operation [, int &$wouldblock]);
Description: PHP supports a portable way of locking complete files
  in an advisory way
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing flock() fun with the various operation and
            wouldblock values                                ***\n";
$file = sys_get_temp_dir().'/'.'flock.tmp';
$fp = fopen($file, "w");

/* array of operatons */
$operations = varray[
  LOCK_SH,
  LOCK_EX,
  LOCK_SH|LOCK_NB,
  LOCK_EX|LOCK_NB,
  LOCK_SH|LOCK_EX,
  LOCK_UN,
  1,
  2,
  2.234,
  TRUE
];

/* array of wouldblocks */
$wouldblocks = varray[
  0,
  1,
  2,
  1.234,
  TRUE,
  FALSE,
  NULL,
  varray[1,2,3],
  varray[],
  "string",
  "",
  /* binary input */
  b"string",
  b"",
  "\0"
];

$i = 0;
foreach($operations as $operation) {
  echo "--- Outer iteration $i ---\n";
  $wouldblock = false;
  var_dump(flock($fp, (int)$operation, inout $wouldblock));
  $j = 0;
  foreach($wouldblocks as $wouldblock) {
    echo "-- Inner iteration $j in $i --\n";
    var_dump(flock($fp, (int)$operation, inout $wouldblock));
    $j++;
  }
  $i++;
}

fclose($fp);
unlink($file);

echo "\n*** Done ***\n";
}
