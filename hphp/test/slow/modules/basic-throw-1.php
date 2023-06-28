<?hh



module A;

<<__EntryPoint>>
function main() :mixed{
  include 'basic-1.inc';
  try {
    Cls::foo_static();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    (new Cls)->foo();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    foo();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    foo();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value("foo")();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value("Cls::foo_static")();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value(vec["Cls", "foo_static"])();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value(vec[new Cls, "foo"])();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    new InternalCls();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $x = __hhvm_intrinsics\launder_value("InternalCls");
  try {
    new $x;
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $x = __hhvm_intrinsics\launder_value(InternalCls::class);
  try {
    new $x;
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    new ReifiedInternalCls();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  $y = __hhvm_intrinsics\launder_value("ReifiedInternalCls");
  try {
    new $y;
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    InternalCls::foo_static();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    $x::foo_static();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value(vec[$x, "foo_static"])();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try {
    __hhvm_intrinsics\launder_value("InternalCls::foo_static")();
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
