<?hh

function main() :mixed{
  echo "Creating closure\n";
  $x = function($throw) {
    echo "Entering closure\n";
    using (new Logger()) {
      echo "In using\n";
      if ($throw) throw new Exception("Closure threw");
    }
    echo "Leaving closure\n";
  };

  $x(false);

  try {
    using (new Logger()) {
      $x(true);
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_closure() :mixed{
require 'logger.inc';

main();
}
