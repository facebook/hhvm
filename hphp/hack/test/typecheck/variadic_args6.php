<?hh

function takes_string(string $s): void {}

function f1(string ...$args): void {
  foreach ($args as $arg) {
    echo $arg;
  }
  takes_string($args); // error, args is array
}
