<?hh

function non_zoned($fn) {
  if ($fn) $fn(null);
}

function zoned_local($fn)[zoned_local] {
  if ($fn) $fn(null);
}

function zoned_shallow($fn)[zoned_shallow] {
  if ($fn) $fn(null);
}

function zoned($fn)[zoned] {
  if ($fn) $fn(null);
}

function zoned_with($fn)[zoned_with] {
  if ($fn) $fn(null);
}

function leak_safe($fn)[leak_safe] {
  if ($fn) $fn(null);
}

function read_globals($fn)[read_globals] {
  if ($fn) $fn(null);
}

function globals($fn)[globals] {
  if ($fn) $fn(null);
}

function write_props($fn)[write_props] {
  if ($fn) $fn(null);
}

function write_this_props($fn)[write_this_props] {
  if ($fn) $fn(null);
}

function pure($fn)[] {
  if ($fn) $fn(null);
}

<<__EntryPoint>>
function main()[zoned_with, defaults] {
  $functions = vec[
    'non_zoned',
    'zoned_local',
    'zoned_shallow',
    'zoned',
    'zoned_with',
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
