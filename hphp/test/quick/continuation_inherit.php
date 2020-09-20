<?hh

function derp() {
  include 'continuation_inherit.inc';
}
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass('Generator');
var_dump($rc->isFinal());

derp();
}
