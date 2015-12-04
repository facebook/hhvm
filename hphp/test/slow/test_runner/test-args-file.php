<?hh

function fibonacci(int $number): int {
  return intval(round(pow((sqrt(5) + 1) / 2, $number) / sqrt(5)));
}

function main(array<string> $argv) {
  echo 'The ' . $argv[1] . ' number in fibonacci is: '
      . fibonacci((int) $argv[1]) . PHP_EOL;
}

main($argv);
