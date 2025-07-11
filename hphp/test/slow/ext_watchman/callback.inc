<?hh

<<__DynamicallyCallable>> function callback_stress(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  $hit_counter = __hhvm_intrinsics\apc_fetch_no_check('stress_counter');
  if ($hit_counter) {
    $hit_counter++;
  } else {
    $hit_counter = 1;
  }
  apc_store('stress_counter', $hit_counter);
}


<<__DynamicallyCallable>> function callback_files(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  require_once 'constants.inc';

  chdir($path);
//  print("DIR: $path, JSON: $data_json\n");
  $data = json_decode($data_json, true);
  if ($data['is_fresh_instance'] ?? false) {
    if (!file_exists(TEST_FILE_GOT_FRESH)) {
      // This will trigger an update
      file_put_contents(TEST_FILE_GOT_FRESH, $data_json);
    }
  } else {
    if (!file_exists(TEST_FILE_GOT_UPDATE)) {
      file_put_contents(TEST_FILE_GOT_UPDATE, $data_json);
    }
  }
}

<<__DynamicallyCallable>> function callback_sync(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  sleep(3);
}

<<__DynamicallyCallable>> function callback_unsubscribe(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  sleep(1);
}

<<__DynamicallyCallable>> function callback_broken(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  $data = json_decode($data_json, true);
  if (array_key_exists('is_fresh_instance', $data)) {
    return;
  }
  if (array_key_exists('connection_error', $data)) {
   apc_store('callback_broken', 'pass');
  }
}

<<__DynamicallyCallable>> function callback_exception(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  throw new Exception("expected exception");
}

<<__DynamicallyCallable>> function callback_checksub(string $sub_name): void {
  // Having these async print statements can be problematic when checking test
  // output so don't include them by default.
  // print("Checking sub: $sub_name\n");
  while (apc_exists('stress_counter')) {
    HH\watchman_check_sub($sub_name);
  }
  // print("Done checking sub: $sub_name\n");
}
