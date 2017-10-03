<?hh

class Logger implements IDisposable, IAsyncDisposable {
  private static $count = 0;
  private $id;

  function __construct($throw = false) {
    $this->id = self::$count++;
    if ($throw) {
      throw new Exception("Logger id ".$this->id);
    }
    printf("Constructing id %d\n", $this->id);
  }

  function __dispose() {
    printf("Disposing id %d\n", $this->id);
  }

  async function __disposeAsync() {
    printf("Async disposing id %d\n", $this->id);
  }

  static async function makeAsync($throw = false) {
    return new Logger($throw);
  }
}
