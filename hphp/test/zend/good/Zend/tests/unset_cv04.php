<?hh
<<__EntryPoint>> function f() :mixed{
  \HH\global_set('x', "ok\n");
  echo \HH\global_get('x');
  include "unset.inc";
  unset_();
  echo \HH\global_get('x');
}
