<?hh

<<__DynamicallyCallable>> function non_rx($fn) :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function rx_local($fn)[rx_local] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function rx_shallow($fn)[rx_shallow] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function rx($fn)[rx] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props($fn)[write_props] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props_rx($fn)[write_props, rx] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props_rx_shallow($fn)[write_props, rx_shallow] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function write_props_rx_local($fn)[write_props, rx_local] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__DynamicallyCallable>> function pure($fn)[] :mixed{
  if ($fn) HH\dynamic_fun($fn)(null);
}

<<__EntryPoint>>
function main() :mixed{
  $functions = vec[
    'non_rx',
    'rx_local',
    'rx_shallow',
    'rx',
    'write_props',
    'write_props_rx',
    'write_props_rx_shallow',
    'write_props_rx_local',
    'pure'
  ];
  foreach ($functions as $caller) {
    foreach ($functions as $callee) {
      try {
        HH\dynamic_fun($caller)($callee);
        echo "$caller -> $callee: ok\n";
      } catch (Exception $e) {
        echo "$caller -> $callee: ".$e->getMessage()."\n";
      }
    }
  }
}
