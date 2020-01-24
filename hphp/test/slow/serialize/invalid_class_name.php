<?hh
<<__EntryPoint>> function main(): void {
$str = 'O:12:"phpinfo();/*":0:{}';
$obj = unserialize($str);
var_dump($obj);

$str = 'C:16:"ChunkySalsa();/*":84:{a:3:{s:7:"storage";a:0:{}s:5:"flags";i:0;s:13:"iteratorClass";s:13:"ArrayIterator";}}';
$obj = unserialize($str);
var_dump($obj);
}
