<?hh

require 'logger.inc';

function main() {
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

main();
