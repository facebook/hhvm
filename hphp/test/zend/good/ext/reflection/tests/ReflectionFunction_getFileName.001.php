<?hh

include "included4.inc";
<<__EntryPoint>> function main(): void {
$funcInfo = new ReflectionFunction('g');
var_dump($funcInfo->getFileName());
}
