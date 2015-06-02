<?hh

$hosts = Vector { getenv('HPHP_TEST_MCROUTER') };
$mcr = MCRouter::createSimple($hosts);
try {
  $got = HH\Asio\join($mcr->get('Utterly Invalid Key'));
  echo "Unexpectedly got a value: ";
  var_dump($got);
} catch (MCRouterException $e) {
  var_dump($e->getMessage());
  var_dump($e->getKey());
  var_dump(MCRouter::getOpName($e->getOp()));
  var_dump(MCRouter::getResultName($e->getCode()));
}
