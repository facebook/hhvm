<?php

echo "no key\n";
$info = apc_cache_info('user');
if (count($info) <= 1) echo "cache size error\n";
if ($info['entries'] != 0) echo "entry count should be 0\n";

apc_add('key', 1);
echo "1 key\n";
$info = apc_cache_info('user');
checkInfo($info, 10, 3, 1, true);
apc_add('key1', array(1, 2, 3));
echo "2 keys\n";
$info = apc_cache_info('user');
checkInfo($info, 32, 7, 2, true);
apc_add('key2', 'hello');
echo "3 keys\n";
$info = apc_cache_info();
checkInfo($info, 48, 10, 3, true);
dumpKeys($info);
apc_delete('key1');
echo "1 delete, 2 keys\n";
$info = apc_cache_info('user');
checkInfo($info, 20, 7, 2, true);
dumpKeys($info);
apc_add('key1', array(true, 10, 4.5678,
                      'hello',
                      array('a' => 'b',
                            'c' => 'd'),
                      'and more'));
echo "add again, 3 keys\n";
$info = apc_cache_info('');
checkInfo($info, 48, 10, 3, true);

class C {}
apc_add('key3', new C);
echo "4 keys\n";
$info = apc_cache_info('user');
checkInfo($info, 90, 14, 4, true);
dumpKeys($info);

$info = apc_cache_info('filehits');
if ($info == null) echo "error with filehits\n";

$info = apc_cache_info('whatever');
if ($info == null) echo "error random cache info\n";

echo "done\n";

// The size values are a bit of a difficult thing to check so we use them as
// sort of a lower bound. It's a weak check but unless it gets bad it's a
// useful thing to check.
function checkInfo($info, $valuesSize, $keysSize, $entriesCount, $checkList) {
  if (count($info) == 1) echo "error\n";
  if ($info['values_size'] < $valuesSize) {
    printf("valuesSize: %d smaller than expected %d\n",
           $info['values_size'], $valuesSize);
  }
  if ($info['keys_size'] < $keysSize) echo "keys size too small\n";
  if ($info['entries'] != $entriesCount) echo "entries count wrong\n";
  if ($checkList && count($info['cache_list']) != $info['entries']) {
    echo "cache list wrong\n";
  }
  if (!$checkList && array_key_exists('cache_list')) {
    echo "cache list should be null\n";
  }
}

function dumpKeys($info) {
  $list = $info['cache_list'];
  foreach($list as $entry) {
    var_dump($entry['entry_name']);
  }
}
