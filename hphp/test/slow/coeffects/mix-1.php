<?hh

function write_props_read_globals($fn)[write_props, read_globals] :mixed{
  if ($fn) $fn(null);
}

function write_props_globals($fn)[write_props, globals] :mixed{
  if ($fn) $fn(null);
}

function defaults($fn)[defaults] :mixed{
  if ($fn) $fn(null);
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
      $caller($callee);
      echo " ok\n";
    }
  }
}
