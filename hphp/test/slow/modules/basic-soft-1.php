<?hh



module A;

<<__EntryPoint>>
function main() :mixed{
  include 'basic-1.inc';
  try {
    (new Cls)->foo();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  (new Cls)->foo_soft();

  try {
    foo();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  foo_soft();

  try {
    new InternalCls();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  new InternalClsSoft();


  $c = new Cls2(1, 1);
  try {
    $c->x = 5;
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $c->x_soft = 5;

  try {
    $c->y = 5;
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $c->y_soft = 5;

  echo "Done\n";
}
