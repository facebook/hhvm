<?hh

<<__EntryPoint>>
function main() :mixed{
  include 'basic-2.inc';
  try { Cls::foo_static(); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { (new Cls)->foo(); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
