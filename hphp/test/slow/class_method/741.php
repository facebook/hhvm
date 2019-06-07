<?hh
class AdsConsoleRenderer {
  public static function getInstance() {
    return new AdsConsoleRenderer();
  }
  function writeMsg($classname = '', $s = '') {
    echo $classname . "::" . $s;
  }
}


<<__EntryPoint>>
function main_741() {
$error = 'fatal error';
echo AdsConsoleRenderer::getInstance()->writeMsg('error', $error);
}
