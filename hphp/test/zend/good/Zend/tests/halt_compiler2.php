<?hh
<<__EntryPoint>>
function main_entry(): void {
  $text = "<?hh echo 'test'; var_dump(__COMPILER_HALT_OFFSET__); __halt_compiler(); ?>
  hi there";
  file_put_contents(dirname(__FILE__) . '/test1.php', $text);
  $text = "<?hh echo 'test2'; var_dump(__COMPILER_HALT_OFFSET__); __halt_compiler(); ?>
  hi there 2";
  file_put_contents(dirname(__FILE__) . '/test2.php', $text);
  include dirname(__FILE__) . '/test1.php';
  include dirname(__FILE__) . '/test2.php';
  echo "==DONE==\n";
  error_reporting(0);
  unlink(dirname(__FILE__) . '/test1.php');
  unlink(dirname(__FILE__) . '/test2.php');
}
