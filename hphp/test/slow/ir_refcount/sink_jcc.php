<?hh

function x() :mixed{ return make_uniqid(microtime()); }

// At the time this test was added, this function could sink increfs into
// conditional exits of a region, which tests jcc service requests that have
// the jcc and the stub in the same code block.
function make_uniqid($prefix = '') :mixed{
  $uniqid = uniqid('', true);
  $uniqid[14] = sprintf("%x", mt_rand(0, 0xF));
  $uniqid = ($prefix ?: 'id_') . $uniqid;
  return $uniqid;
}


<<__EntryPoint>>
function main_sink_jcc() :mixed{
x();x();x();x();x();x();x();
}
