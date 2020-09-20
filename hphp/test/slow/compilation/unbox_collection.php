<?hh

function foo($feature1) {
  $feats = Vector{$feature1};
  sort(inout $feats);
  return $feats[0];
}


<<__EntryPoint>>
function main_unbox_collection() {
var_dump(foo('foo'));
}
