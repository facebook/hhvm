<?hh


<<__EntryPoint>>
function main_1763() {
error_reporting(0);
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', varray[0]));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', varray[0], 3));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', varray[0], 1.0));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', varray[0], null));
$obj = new stdClass();
try {
  var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', 0, $obj));
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0', '1.0'));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0.0', 1.0));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0.0', 1));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', 0.0, '1'));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob',                        varray[0], varray[1]));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', varray['bob'],                        varray[0], varray[3,4]));
var_dump(substr_replace('ABCDEFGH:/MNRPQR/', varray['bob'],                        varray[0], varray[3]));
var_dump(substr_replace(varray['ABCDEFGH:/MNRPQR/'], varray[],                        varray[0,1], varray[3, 4]));
var_dump(substr_replace(varray['ABCDEFGH:/MNRPQR/'], varray['bob'],                        varray[0,1], varray[3]));
var_dump(substr_replace(varray['ABCDEFGH:/MNRPQR/'],                        varray['bob', 'cat'], 0));
var_dump(substr_replace(varray['ABCDEFGH:/MNRPQR/'],                        varray['bob'], varray[0,1]));
var_dump(substr_replace('abc', 'xyz', 3, 0));
var_dump(sscanf("SN/2350001", "SN/%d"));
var_dump(sscanf("SN/abc", "SN/%d"));
var_dump(sscanf("30", "%da"));
var_dump(sscanf("-", "%da"));
}
