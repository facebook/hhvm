<?hh

class FOo {
  public function exclAIM() {
 echo "FOo::exclAIM\n";
 }
  public function teST() {
 echo "FOo::teST\n";
 }
  public function __FUNcTION__() {
 echo "FOo::__FUNcTION__\n";
 }
  public function __CLAsS__() {
 echo "FOo::__CLAsS__\n";
 }
}
class TesT {
  public static function ExclAim() {
    $obj = new fOO();
    $obj->{
__FUNCTION__}
 = 1;
    $obj->{
__CLASS__}
 = 2;
    $obj->__FuNCTION__ = 3;
    $obj->__ClASS__ = 4;
    $obj->{
__FUNCTION__}
();
    $obj->{
__CLASS__}
();
    $obj->__FUNcTION__();
    $obj->__CLAsS__();
    $arr = darray[];
    foreach ($obj as $k => $v) {
      $arr[$k] = $v;
    }
    ksort(inout $arr);
    var_dump($arr);
  }
}

<<__EntryPoint>>
function main_672() {
tEst::eXclaiM();
}
