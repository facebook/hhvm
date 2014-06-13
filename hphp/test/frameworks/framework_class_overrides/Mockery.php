<?hh
require_once __DIR__.'/../Framework.php';

class Mockery extends Framework {
  public function __construct(string $name) {
    /* Mockery tries stubbing out part of Mongo, which we have
     * in Zend compat, but it's not at a usable state yet.'
     */
    $tc = get_runtime_build().' -c '.__DIR__.'/Mockery.ini'.
          ' '.__DIR__.'/../vendor/bin/phpunit';
    parent::__construct($name, $tc);
  }
}
