<?hh

function non_rx($fn) :mixed{
  if ($fn) $fn(null);
}

function rx_local($fn)[rx_local] :mixed{
  if ($fn) $fn(null);
}

function rx_shallow($fn)[rx_shallow] :mixed{
  if ($fn) $fn(null);
}

function rx($fn)[rx] :mixed{
  if ($fn) $fn(null);
}

function write_props($fn)[write_props] :mixed{
  if ($fn) $fn(null);
}

function write_props_rx($fn)[write_props, rx] :mixed{
  if ($fn) $fn(null);
}

function write_props_rx_shallow($fn)[write_props, rx_shallow] :mixed{
  if ($fn) $fn(null);
}

function write_props_rx_local($fn)[write_props, rx_local] :mixed{
  if ($fn) $fn(null);
}

function pure($fn)[] :mixed{
  if ($fn) $fn(null);
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
        $caller($callee);
        echo "$caller -> $callee: ok\n";
      } catch (Exception $e) {
        echo "$caller -> $callee: ".$e->getMessage()."\n";
      }
    }
  }
}
