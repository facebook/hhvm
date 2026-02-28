<?hh

trait PropertiesTrait {
   public static $same = true;
}

class Properties {
   use PropertiesTrait;
   public $same = true;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
