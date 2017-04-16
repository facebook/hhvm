<?hh
require_once __DIR__.'/../Framework.php';

class Doctrine2 extends Framework {
  public function __construct(string $name) {
    /* Doctrine2 tries to use the protocol 'file' for
     * external XML entities so we must enable that.
     */
    $tc = get_runtime_build().' -c '.__DIR__.'/Doctrine2.ini'.
          ' '.__DIR__.'/../vendor/bin/phpunit';
    parent::__construct($name, $tc);
  }
}
