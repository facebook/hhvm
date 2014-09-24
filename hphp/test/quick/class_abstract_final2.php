<?hh

abstract final class Utils {
  public static function staticMethod() {
    echo __METHOD__, "\n";
  }

  public function noGood() {}
}

function main() {
  Utils::staticMethod();
}

main();
echo 'Done', "\n";
