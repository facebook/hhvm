<?hh

function foo($feature1) {
  $feats = Vector{$feature1};
  sort(&$feats);
  return $feats[0];
}


<<__EntryPoint>>
function main_unbox_collection() {
var_dump(foo('foo'));
}
