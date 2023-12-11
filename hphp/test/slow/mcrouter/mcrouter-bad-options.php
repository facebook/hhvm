<?hh
<<__EntryPoint>> function main(): void {
try {
  $mcr = new MCRouter(dict[
    'asynclog_disable' => 'purple',
  ]);
  var_dump($mcr);
} catch (MCRouterOptionException $e) {
  var_dump($e->getMessage());
  var_dump($e->getErrors());
}
}
