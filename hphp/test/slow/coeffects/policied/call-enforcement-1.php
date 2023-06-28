<?hh

function non_zoned($fn) :mixed{
  if ($fn) $fn(null);
}

function zoned_local($fn)[zoned_local] :mixed{
  if ($fn) $fn(null);
}

function zoned_shallow($fn)[zoned_shallow] :mixed{
  if ($fn) $fn(null);
}

function zoned($fn)[zoned] :mixed{
  if ($fn) $fn(null);
}

function zoned_with($fn)[zoned_with] :mixed{
  if ($fn) $fn(null);
}

function leak_safe_local($fn)[leak_safe_local] :mixed{
  if ($fn) $fn(null);
}

function leak_safe_shallow($fn)[leak_safe_shallow] :mixed{
  if ($fn) $fn(null);
}

function leak_safe($fn)[leak_safe] :mixed{
  if ($fn) $fn(null);
}

function read_globals($fn)[read_globals] :mixed{
  if ($fn) $fn(null);
}

function globals($fn)[globals] :mixed{
  if ($fn) $fn(null);
}

function write_props($fn)[write_props] :mixed{
  if ($fn) $fn(null);
}

function write_this_props($fn)[write_this_props] :mixed{
  if ($fn) $fn(null);
}

function pure($fn)[] :mixed{
  if ($fn) $fn(null);
}

<<__EntryPoint>>
function main()[zoned_with, defaults] :mixed{
  $functions = vec[
    'non_zoned',
    'zoned_local',
    'zoned_shallow',
    'zoned',
    'zoned_with',
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
      $caller($callee);
      echo " ok\n";
    }
  }
}
