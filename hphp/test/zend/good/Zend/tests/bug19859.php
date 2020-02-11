<?hh
class test
{
  function __call($method,$args)
  {
    print "test::__call invoked for method '$method'\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new test;
$x->fake(1);
call_user_func_array(varray[$x,'fake'],varray[1]);
call_user_func(varray[$x,'fake'],2);
}
