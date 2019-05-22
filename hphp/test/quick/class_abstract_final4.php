<?hh

abstract final class Utils {
  public static function staticMethod() {
    echo __METHOD__, "\n";
  }

  public $noGood;
}

function main() {
  Utils::staticMethod();
}
<<__EntryPoint>> function main_entry(): void {
main();
echo 'Done', "\n";
}
