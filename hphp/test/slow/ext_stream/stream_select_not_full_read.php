<?hh


<<__EntryPoint>>
function main_stream_select_not_full_read() :mixed{
$descriptorspec = vec[
  vec["pipe", "r"],
  vec["pipe", "w"],
  vec["pipe", "a"],
];
$io = null;
$process = proc_open('cat', darray($descriptorspec), inout $io);
fwrite($io[0], "a\nb\nc\n");

// Just to have another thing in the $r array that has no data
$io2 = null;
$process2 = proc_open('cat', darray($descriptorspec), inout $io2);

while (!feof($io[1])) {
  $r = vec[$io[1], $io2[1]];
  $w = $e = null;
  $i = stream_select(inout $r, inout $w, inout $e, 1);
  var_dump($i);
  if ($i) {
    foreach ($r as $resource) {
      var_dump(fgets($resource));
    }
  } else {
    break;
  }
}
}
