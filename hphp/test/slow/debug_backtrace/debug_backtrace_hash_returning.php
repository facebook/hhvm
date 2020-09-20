<?hh

function call_profile_hook_upon_exit() {
  echo "going to call profile hook\n";
}

function set_metadata(string $metadata) {
  HH\set_frame_metadata($metadata);
  call_profile_hook_upon_exit();
}

function get_metadata_hash() {
  echo "getting metadata\n";
  hphp_debug_backtrace_hash(DEBUG_BACKTRACE_HASH_CONSIDER_METADATA);
}

<<__EntryPoint>>
function main() {
  fb_setprofile(($case, $fn) ==> {
    if ($case === 'exit' && $fn === 'call_profile_hook_upon_exit') {
      get_metadata_hash();
    }
  });

  set_metadata('hello world');
  echo "survived\n";
}
