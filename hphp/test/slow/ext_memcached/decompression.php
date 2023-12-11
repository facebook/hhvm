<?hh
const MEMC_SERIALIZED =          4;
const MEMC_COMPRESSED =         16;
const MEMC_COMPRESSION_ZLIB =   32;
const MEMC_COMPRESSION_FASTLZ = 64;
<<__EntryPoint>> function main(): void {
$keys = dict[
  'no_compression'   => dict[
    'flag' => MEMC_SERIALIZED,
    'data' => 'a:2:{s:16:"compression_type";N;s:5:"valid";b:1;}',
  ],
  'old_style_zlib'   => dict[
    'flag' => MEMC_SERIALIZED | MEMC_COMPRESSED,
    'data' =>
     base64_decode('eJxLtDKyqi62MjSzUkrOzy0oSi0uzszPiy+pLEhVsgaKm1gp5eekxBeXVOa'
                    . 'kxlflZCaBRE2tlMoSczJTlKyTrAytawEh2Bb9'),
  ],
  'new_style_zlib'   => dict[
    'flag' => MEMC_SERIALIZED | MEMC_COMPRESSED | MEMC_COMPRESSION_ZLIB,
    'data' =>
     base64_decode('RAAAAHicS7QysqoutjI0s1JKzs8tKEotLs7Mz4svqSxIVbIGiptYKeWllsc'
                    . 'Xl1TmpMZX5WQmgURNrZTKEnMyU5Ssk6wMrWsBIyQXCA=='),
  ],
  'new_style_fastlz' => dict[
    'flag' => MEMC_SERIALIZED | MEMC_COMPRESSED | MEMC_COMPRESSION_FASTLZ,
    'data' =>
     base64_decode('RgAAABxhOjI6e3M6MTY6ImNvbXByZXNzaW9uX3R5cGUiO4AXD25ld19zdHl'
                    . 'sZV9mYXN0bHpAFw41OiJ2YWxpZCI7YjoxO30='),
  ],
];

$errno = null;
$errstr = null;
// Write the values to memcache using a raw socket connection
// to make sure that they are not transformed in any way.
$socket = fsockopen('localhost', 11211, inout $errno, inout $errstr);
$socket || die("Couldn't connect to memcache.\n");
foreach($keys as $key => $value) {
  extract($value, EXTR_OVERWRITE);
  $size = strlen($data);
  fwrite($socket, "set $key $flag 0 $size\r\n$data\r\n");
  fread($socket, 8) === "STORED\r\n" || die("Couldn't store value.\n");
}
socket_close($socket);

// Read the values from memcached and decompress/unserialize.
$mc = new Memcached;
$mc->addServer('localhost', 11211);
foreach($keys as $key => $value) {
  var_dump($mc->get($key));
}
}
