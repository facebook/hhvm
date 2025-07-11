<?hh

<<__DynamicallyCallable>> function non_zoned($fn) :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function zoned_local($fn)[zoned_local] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function zoned_shallow($fn)[zoned_shallow] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function zoned($fn)[zoned] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function leak_safe_local($fn)[leak_safe_local] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function leak_safe_shallow($fn)[leak_safe_shallow] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function leak_safe($fn)[leak_safe] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function read_globals($fn)[read_globals] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function globals($fn)[globals] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props($fn)[write_props] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_this_props($fn)[write_this_props] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function pure($fn)[] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__EntryPoint>>
function main()[defaults] :mixed{
  $functions = vec[
    'non_zoned',
    'zoned_local',
    'zoned_shallow',
    'zoned',
    'leak_safe_local',
    'leak_safe_shallow',
    'leak_safe',
    'globals',
    'read_globals',
    'write_props',
    'write_this_props',
    'pure'
  ];
  foreach ($functions as $caller) {
    foreach ($functions as $callee) {
      echo "$caller -> $callee:";
      HH\dynamic_fun($caller)($callee);
      echo " ok\n";
    }
  }
}
