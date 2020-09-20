<?hh

function starts_with(
  $big_string,
  $little_string,
  $case_insensitive=false): bool {

    $len = strlen($little_string);
    return !$len ||
      0 ===
      (
        $case_insensitive
        ? strncasecmp(
          $big_string,
          $little_string,
          $len,
        )
        : (
          $big_string
          ? strncmp(
            $big_string,
            $little_string,
            $len,
          )
          : null
        )
      );
}
<<__EntryPoint>> function main(): void {
var_dump(starts_with("foobar", "Foo", true));
}
