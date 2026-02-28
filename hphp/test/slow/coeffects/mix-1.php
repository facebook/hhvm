<?hh

<<__DynamicallyCallable>> function write_props_read_globals($fn)[write_props, read_globals] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props_globals($fn)[write_props, globals] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function defaults($fn)[defaults] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__EntryPoint>>
function main() :mixed{
  $functions = vec[
    'write_props_read_globals',
    'write_props_globals',
    'defaults',
  ];
  foreach ($functions as $caller) {
    foreach ($functions as $callee) {
      echo "$caller -> $callee:";
      HH\dynamic_fun($caller)($callee);
      echo " ok\n";
    }
  }
}
