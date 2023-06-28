<?hh


<<__EntryPoint>>
function main_unserialize_map() :mixed{
$s = 'K:6:"HH\Map":4:{i:1;s:3:"foo";i:1;s:3:"bar";s:1:"a";i:1;s:1:"a";i:2;}';
var_dump(unserialize($s));
}
