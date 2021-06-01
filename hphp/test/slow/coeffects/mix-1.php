<?hh

function write_props_read_globals($fn)[write_props, read_globals] {
  if ($fn) $fn(null);
}

function write_props_globals($fn)[write_props, globals] {
  if ($fn) $fn(null);
}

function defaults($fn)[defaults] {
  if ($fn) $fn(null);
}

<<__EntryPoint>>
function main() {
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
