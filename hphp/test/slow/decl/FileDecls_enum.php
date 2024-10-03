<?hh

<<__EntryPoint>>
function main(): void {
  // Get the autoloader to initialize
  HH\autoload_is_native();

  $text = '<?hh
    enum E: int {
      A = "X";
      B = nameof X;
      C = X::class;
    }
  ';

    $instance = HH\FileDecls::parseText($text);
    $consts = $instance->getClass('E')["consts"];
    var_dump($consts[0]);
    var_dump($consts[1]);
    var_dump($consts[2]);
}
