<?hh

function examine($a) :mixed{
  try { print $a[0]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a[1]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a["abc"]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a["def"]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[0] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[1] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a["abc"] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a["def"] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[] = " a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

}

function examine_io(inout $a) :mixed{
  try { print $a[0]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a[1]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a["abc"]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { print $a["def"]."\n"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[0] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[1] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a["abc"] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a["def"] = "a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

  try { $a[] = " a"; }
  catch(Exception $e) { echo get_class($e).": ".$e->getMessage()."\n"; }

}

<<__EntryPoint>>
function main() :mixed{
  print "vec:\n";
  examine(vec["test"]);
  $a = vec["test"];
  examine_io(inout $a);
  print "varray:\n";
  examine(vec["test"]);
  $a = vec["test"];
  examine_io(inout $a);
  print "dict:\n";
  examine(dict[0 => "test"]);
  $a = dict[0 => "test"];
  examine_io(inout $a);
  print "darray:\n";
  examine(dict[0 => "test"]);
  $a = dict[0 => "test"];
  examine_io(inout $a);
  print "keyset:\n";
  examine(keyset[0]);
  $a = keyset[0];
  examine_io(inout $a);
}
