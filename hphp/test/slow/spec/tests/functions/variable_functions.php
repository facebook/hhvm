<?hh

<<__DynamicallyCallable>>
function f1()
:mixed{
    echo "Inside function " . __FUNCTION__ . "\n";
}
//'\\NS1\\f2'();    // can't be a literal
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  include_once 'TestInc.inc'; // get access to \NS1\f2()

  f1();
  \f1();
  namespace\f1();

  $v = 'f1';
  $v();
  $v = '\\f1';
  $v();
  //'f1'();           // can't be a literal

  $v = '\\NS1\\f2';
  $v();
}
