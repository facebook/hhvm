<?hh

<<__EntryPoint>> function main(): void {
  echo 'Executable lines on trait' . PHP_EOL;
  var_dump(
    HH\get_executable_lines(__DIR__ . '/exec_lines_trait.inc')
  );
}
