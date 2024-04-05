<?hh

<<__EntryPoint>> function main(): void {
  echo 'Executable lines on abstract class' . PHP_EOL;
  var_dump(
    HH\get_executable_lines(__DIR__ . '/exec_lines_abstract_class.inc')
  );
}
