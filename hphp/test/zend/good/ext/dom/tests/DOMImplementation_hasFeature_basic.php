<?hh <<__EntryPoint>> function main() {
$imp = new DOMImplementation();
var_dump($imp->hasFeature('Core', '1.0'));
var_dump($imp->hasFeature('XML', '2.0'));
}
