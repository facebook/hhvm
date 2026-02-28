<?hh

function derp() :mixed{
  include 'continuation_inherit.inc';
}
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass('Generator');
var_dump($rc->isFinal());

derp();
}
