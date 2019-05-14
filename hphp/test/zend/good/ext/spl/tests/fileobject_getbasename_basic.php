<?php <<__EntryPoint>> function main() {
$file = __FILE__;
$s = new SplFileObject( __FILE__ );
echo $s->getBasename();
}
