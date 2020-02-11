<?hh <<__EntryPoint>> function main(): void {
error_reporting (E_ALL);
$a      = varray[4];
$str    = serialize($a);
print('Serialized array: '.$str."\n");
$b      = unserialize($str);
print('Unserialized array: ');
var_dump($b);
print("\n");
$str    = serialize(varray[4.5]);
print('Serialized array: '.$str."\n");
$b      = unserialize($str);
print('Unserialized array: ')   ;
var_dump($b);
}
