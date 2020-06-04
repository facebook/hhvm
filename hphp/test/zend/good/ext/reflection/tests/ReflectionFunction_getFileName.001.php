<?hh
<<__EntryPoint>> function main(): void {
include "included4.inc";
$funcInfo = new ReflectionFunction('g');
var_dump($funcInfo->getFileName());
}
