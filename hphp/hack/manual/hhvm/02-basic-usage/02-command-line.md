# Command Line

In command-line (cli) mode, you run the `hhvm` binary from the command-line, execute the script and then exit HHVM immediately when the script completes.

Here is an example of how to run a script in HHVM cli mode. Take the following Hack script:

```fib.hack
function fibonacci(int $number): int {
  return \intval(\round(\pow((\sqrt(5.0) + 1) / 2, $number) / \sqrt(5.0)));
}

<<__EntryPoint>>
function main(): void {
  $n = (int) (vec(\HH\global_get('argv') as Container<_>)[1] ?? 10);
  echo 'The '.
    $n.
    ' number in fibonacci is: '.
    fibonacci($n).
    \PHP_EOL;
}
```

At the command-line, you would execute the script as follows:

```
$ hhvm /path/to/fib.hack 10
```

You specify the `hhvm` binary, the path to `fib.hack` and an argument to the script (arguments to scripts do not exist in all cases, of course).
