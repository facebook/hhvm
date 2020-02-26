<?hh <<__EntryPoint>> function main(): void {
$arr = varray['foo','bar','baz'];
$foo = 'foo';
$bar = 'bar';
$baz = 'baz';
var_dump($arr[0]);
var_dump($arr[1]);
var_dump($arr[2]);
var_dump(varray['foo','bar','baz'][0]);
var_dump(varray['foo','bar','baz'][1]);
var_dump(varray['foo','bar','baz'][2]);
var_dump(varray[$foo,$bar,$baz][0]);
var_dump(varray[$foo,$bar,$baz][1]);
var_dump(varray[$foo,$bar,$baz][2]);
$arr2 = varray['blah'];
}
