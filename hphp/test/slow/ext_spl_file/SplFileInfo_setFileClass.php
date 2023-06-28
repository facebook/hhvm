<?hh

class MyFileObject extends SplFileObject {}


<<__EntryPoint>>
function main_spl_file_info_set_file_class() :mixed{
$info = new SplFileInfo(__FILE__);

$info->setFileClass('MyFileObject');
echo get_class($info->openFile()), "\n";

$info->setFileClass('SplFileObject');
echo get_class($info->openFile()), "\n";

try {
    $info->setFileClass('stdClass');
} catch (UnexpectedValueException $e) {
    echo $e->getMessage(), "\n";
}
}
