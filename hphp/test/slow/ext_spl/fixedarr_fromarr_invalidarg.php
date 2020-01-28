<?hh


<<__EntryPoint>>
function main_fixedarr_fromarr_invalidarg() {
try {
  SplFixedArray::fromArray(darray['string'=>'string']);
} catch (InvalidArgumentException $e) {
  echo $e->getMessage();
  echo "\n";
}
}
