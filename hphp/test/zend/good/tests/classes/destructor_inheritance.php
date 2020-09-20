<?hh
class base {
   function __construct() {
      echo __METHOD__ . "\n";
   }
}

class derived extends base {
}
<<__EntryPoint>> function main(): void {
$obj = new derived;

unset($obj);

echo 'Done';
}
