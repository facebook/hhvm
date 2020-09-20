<?hh <<__EntryPoint>> function main(): void {
$file = dirname(__FILE__) .'/simpletext.wbmp';
jpeg2wbmp('', $file, 20, 120, 8);
error_reporting(0);
unlink(dirname(__FILE__) .'/simpletext.wbmp');
}
