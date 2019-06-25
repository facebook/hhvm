<?hh

trait PropertiesTrait {
   static $same = true;
}

class Properties {
   use PropertiesTrait;
   public $same = true;
}

<<__EntryPoint>> function main(): void {}
