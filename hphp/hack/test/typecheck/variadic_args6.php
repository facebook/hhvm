<?hh

function takes_string(string $s): void {}

function f1(...$args): void {
  // hh_show($args);
  foreach ($args as $arg) {
    echo $arg;
  }
  takes_string($args); // should be an error
}
