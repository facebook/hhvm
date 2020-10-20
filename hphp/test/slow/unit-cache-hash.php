<?hh

function temp_directory() {
  if ($_ENV['HPHP_TEST_TMPDIR'] ?? false) {
    return $_ENV['HPHP_TEST_TMPDIR'];
  }
  return sys_get_temp_dir();
}

function temp_filename($prefix) {
  return tempnam(temp_directory(), "unit-cache-hash-$prefix-");
}

const NUM_FILES = 3;

function setup() {
  $v = vec[];
  for ($i = 0; $i < NUM_FILES; ++$i) {
    $f = temp_filename($i);
    create_file($f);
    $v[] = $f;
  }
  apc_store('filenames', $v);
}

function get_count() {
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) {
    $count = 0;
    apc_store('count', $count);
  }
  return $count;
}

function fetch_filename_for_count($count) {
  $filenames = __hhvm_intrinsics\apc_fetch_no_check('filenames');
  if ($filenames === false || $count >= count($filenames)) return null;
  return $filenames[$count];
}

function sanitize_filename($filename) {
  if ($filename === '') return '';
  $filenames = __hhvm_intrinsics\apc_fetch_no_check('filenames_sanitize');
  $next_filename = __hhvm_intrinsics\apc_fetch_no_check('next_filename');
  if ($filenames === false) $filenames = dict[];
  if ($next_filename === false) $next_filename = 0;
  if (array_key_exists($filename, $filenames)) {
    return $filenames[$filename];
  }
  $new_filename = "file-$next_filename";
  $filenames[$filename] = $new_filename;
  echo "$filename ==> $new_filename\n";
  ++$next_filename;
  apc_store('filenames_sanitize', $filenames);
  apc_store('next_filename', $next_filename);
  return $new_filename;
}

function sanitize_address($addr) {
  $addresses = __hhvm_intrinsics\apc_fetch_no_check('addresses');
  $next_addr = __hhvm_intrinsics\apc_fetch_no_check('next_addr');
  if ($addresses === false) $addresses = dict[];
  if ($next_addr === false) $next_addr = 0;
  if (array_key_exists($addr, $addresses)) {
    return $addresses[$addr];
  }
  $new_addr = $next_addr;
  $addresses[$addr] = $new_addr;
  ++$next_addr;
  apc_store('addresses', $addresses);
  apc_store('next_addr', $next_addr);
  return $new_addr;
}

function sanitize_hash($hash) {
  $hashes = __hhvm_intrinsics\apc_fetch_no_check('hashes');
  $next_hash = __hhvm_intrinsics\apc_fetch_no_check('next_hash');
  if ($hashes === false) $hashes = dict[];
  if ($next_hash === false) $next_hash = 0;
  if (array_key_exists($hash, $hashes)) {
    return (string)$hashes[$hash];
  }
  $new_hash = $next_hash;
  $hashes[$hash] = $new_hash;
  ++$next_hash;
  apc_store('hashes', $hashes);
  apc_store('next_hash', $next_hash);
  return (string)$new_hash;
}

function sanitize($info) {
  $paths = $info['path-to-unit'];
  $new_paths = vec[];
  foreach ($paths as $p) {
    $p['addr'] = sanitize_address($p['addr']);
    $p['sha1'] = sanitize_hash($p['sha1']);
    $p['bc-sha1'] = sanitize_hash($p['bc-sha1']);
    $p['path'] = sanitize_filename($p['path']);
    $p['orig-filepath'] = sanitize_filename($p['orig-filepath']);
    $p['per-request-filepath'] = sanitize_filename($p['per-request-filepath']);
    $new_paths[] = $p;
  }

  $hashes = $info['hash-to-unit'];
  $new_hashes = vec[];
  foreach ($hashes as $h) {
    $h['addr'] = sanitize_address($h['addr']);
    $h['sha1'] = sanitize_hash($h['sha1']);
    $h['bc-sha1'] = sanitize_hash($h['bc-sha1']);
    $h['orig-filepath'] = sanitize_filename($h['orig-filepath']);
    $h['per-request-filepath'] = sanitize_filename($h['per-request-filepath']);
    $new_hashes []= $h;
  }

  usort(inout $new_paths, function($a,$b) {
    if ($a['path'] < $b['path']) return -1;
    if ($a['path'] > $b['path']) return 1;
    return 0;
  });
  usort(inout $new_hashes, function($a,$b) {
    if ($a['orig-filepath'] < $b['orig-filepath']) return -1;
    if ($a['orig-filepath'] > $b['orig-filepath']) return 1;
    if ($a['addr'] < $b['addr']) return -1;
    if ($a['addr'] > $b['addr']) return 1;
    return 0;
  });

  return dict['path-to-unit' => $new_paths, 'hash-to-unit' => $new_hashes];
}

<<__EntryPoint>>
function test() {
  $count = get_count();
  if ($count == 0) {
    echo "Setting up...\n";
    setup();
  } else if ($count < NUM_FILES+1) {
    $filename = fetch_filename_for_count($count - 1);
    if ($filename === null) return;
    echo "Creating $filename...\n";
    require $filename;
    say_file();
  } else if ($count < NUM_FILES*2 + 1) {
    $filename = fetch_filename_for_count($count - NUM_FILES - 1);
    if ($filename === null) return;
    echo "Touching $filename...\n";
    create_file($filename);
    require $filename;
    say_file();
  } else if ($count < NUM_FILES*3 + 1) {
    $filename = fetch_filename_for_count($count - NUM_FILES*2 - 1);
    if ($filename === null) return;
    echo "Modifying to $filename...\n";
    modify_file($filename);
    require $filename;
    say_file2();
  } else {
    echo "Letting treadmill run...\n";
  }

  var_dump(sanitize(__hhvm_intrinsics\non_repo_unit_cache_info()));
  apc_store('count', $count+1);
}

function create_file($filename) {
  $contents = <<<EOT
<?hh
function say_file() { var_dump(__FILE__); }

EOT;

  file_put_contents($filename, $contents);
}

function modify_file($filename) {
  $contents = <<<EOT
<?hh
function say_file2() { var_dump(__FILE__); }
EOT;

  file_put_contents($filename, $contents);
}
