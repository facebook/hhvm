<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(defined('HHVM_VERSION'));
  var_dump(defined('HHVM_VERSION_MAJOR'));
  var_dump(defined('HHVM_VERSION_MINOR'));
  var_dump(defined('HHVM_VERSION_PATCH'));
  var_dump(defined('HHVM_VERSION_ID'));
  var_dump(is_int(HHVM_VERSION_MAJOR));
  var_dump(is_int(HHVM_VERSION_MINOR));
  var_dump(is_int(HHVM_VERSION_PATCH));
  var_dump(is_int(HHVM_VERSION_ID));

  $version_int =
    HHVM_VERSION_PATCH +
    HHVM_VERSION_MINOR * 100 +
    HHVM_VERSION_MAJOR * 100000;
  if (HHVM_VERSION_ID === $version_int) {
    echo "HHVM_VERSION_ID matches HHVM_VERSION_{MAJOR,MINOR,PATCH}\n";
  } else {
    printf(
      "HHVM_VERSION_ID (%d) does NOT match HHVM_VERSION_{MAJOR,MINOR,PATCH} (%d)\n",
      HHVM_VERSION_ID,
      $version_id,
    );
  }

  $version_str = sprintf('%d.%d.%d', HHVM_VERSION_MAJOR, HHVM_VERSION_MINOR, HHVM_VERSION_PATCH);
  if (HHVM_VERSION === $version_str || HHVM_VERSION === $version_str.'-dev') {
    echo "HHVM_VERSION matches HHVM_VERSION_{MAJOR,MINOR,PATCH}\n";
  } else {
    printf(
      "HHVM_VERSION (%s) does NOT match HHVM_VERSION_{MAJOR,MINOR,PATCH} (%s)\n",
      HHVM_VERSION,
      $version_str,
    );
  }
}
