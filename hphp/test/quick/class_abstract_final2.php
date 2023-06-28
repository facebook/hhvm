<?hh

abstract final class Utils {
  public static function staticMethod() :mixed{
    echo __METHOD__, "\n";
  }

  public function noGood() :mixed{}
}

function main() :mixed{
  Utils::staticMethod();
}
<<__EntryPoint>> function main_entry(): void {
main();
echo 'Done', "\n";
}
