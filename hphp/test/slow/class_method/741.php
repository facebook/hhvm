<?hh
class AdsConsoleRenderer {
  public static function getInstance() :mixed{
    return new AdsConsoleRenderer();
  }
  function writeMsg($classname = '', $s = '') :mixed{
    echo $classname . "::" . $s;
  }
}


<<__EntryPoint>>
function main_741() :mixed{
$error = 'fatal error';
echo AdsConsoleRenderer::getInstance()->writeMsg('error', $error);
}
