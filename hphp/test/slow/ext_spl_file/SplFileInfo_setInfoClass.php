<?hh

class MyInfoObject extends SplFileInfo {}


<<__EntryPoint>>
function main_spl_file_info_set_info_class() :mixed{
$info = new SplFileInfo(__FILE__);

$info->setInfoClass('MyInfoObject');
echo get_class($info->getFileInfo()), "\n";
echo get_class($info->getPathInfo()), "\n";

$info->setInfoClass('SplFileInfo');
echo get_class($info->getFileInfo()), "\n";
echo get_class($info->getPathInfo()), "\n";

try {
    $info->setInfoClass('stdClass');
} catch (UnexpectedValueException $e) {
    echo $e->getMessage(), "\n";
}
}
