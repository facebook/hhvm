<?hh


<<__EntryPoint>>
function main_gmt_offsets() :mixed{
$tzs = vec[
  'GMT+0',
  'GMT-1',
  'GMT+1',
  'Etc/GMT+0',
  'Etc/GMT+1',
  'Etc/GMT-1',
];

foreach ($tzs as $tz) {
  var_dump(new DateTimeZone($tz));
}

print("---Repeating to check caching---\n");

foreach ($tzs as $tz) {
  var_dump(new DateTimeZone($tz));
}
}
