<?hh

try {
  $mcr = new MCRouter(array(
    'asynclog_disable' => 'purple',
  ));
  var_dump($mcr);
} catch (MCRouterOptionException $e) {
  var_dump($e->getMessage());
  var_dump($e->getErrors());
}
