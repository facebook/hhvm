<?hh

$hosts = Vector { getenv('HPHP_TEST_MCROUTER') };
$mcr = MCRouter::createSimple($hosts);
try {
  $got = $mcr->get('Utterly Invalid Key')->join();
  echo "Unexpectedly got a value: ";
  var_dump($got);
} catch (MCRouterException $e) {
  var_dump($e->getMessage());
  var_dump($e->getKey());
  var_dump(MCRouter::getOpName($e->getOp()));
  var_dump(MCRouter::getResultName($e->getCode()));
}
