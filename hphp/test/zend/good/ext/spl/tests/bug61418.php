<?hh <<__EntryPoint>> function main(): void {
$fileIterator = new FilesystemIterator(__DIR__, FilesystemIterator::KEY_AS_FILENAME);
$regexpIterator = new RegexIterator($fileIterator, '#.*#');
foreach ($fileIterator as $key => $file)
{
}
$regexpIterator = null;
$fileIterator = null;

$dirIterator = new DirectoryIterator(__DIR__);
$regexpIterator2 = new RegexIterator($dirIterator, '#.*#');
foreach ($dirIterator as $key => $file)
{
}
$regexpIterator2 = null;
$dirIterator = null;
echo "==DONE==";
}
