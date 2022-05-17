<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

<<__EntryPoint>>
function main() {
  include 'basic-1.inc';
  try { Cls::foo_static(); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { (new Cls)->foo(); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
