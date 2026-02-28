<?hh

class Logger implements IDisposable {
  public static $nextId = 0;
  private $id;

  function __construct() {
    $this->id = self::$nextId++;
    printf("Logger %d constructing\n", $this->id);
  }
  function __dispose() :mixed{
    printf("Logger %d disposing\n", $this->id);
  }
}
function main() :mixed{
  echo "Entering loop\n";
  $loop = true;
  while (true) {
    using (new Logger()) {
      using (new Logger()) {
        $loop__str = (string)($loop);
        echo "loop: $loop__str\n";
        if ($loop) {
          $loop = false;
          continue;
        }
      }
    }

    using (new Logger()) {
      echo "In using\n";
      break;
      echo "Shouldn't print\n";
    }
    echo "Also shouldn't print\n";
  }
  echo "After loop\n";

  using (new Logger()) {
    for ($i = 0; $i < 5; ++$i) {
      echo "Starting $i\n";
      if ($i == 2) continue;
      if ($i == 4) break;
      echo "Ending $i\n";
    }
  }
  echo "Done with main\n";
}

<<__EntryPoint>>
function main_loops() :mixed{
main();
}
