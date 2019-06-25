<?hh
class SXE extends SimpleXmlElement {
    public function count() {
        echo "Called Count!\n";
        return parent::count();
    }
}
<<__EntryPoint>> function main(): void {
$str = '<xml><c>asdf</c><c>ghjk</c></xml>';
$sxe = new SXE($str);
var_dump(count($sxe));
echo "==Done==";
}
