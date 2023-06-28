<?hh

function foo($feature1) :mixed{
  $feats = Vector{$feature1};
  sort(inout $feats);
  return $feats[0];
}


<<__EntryPoint>>
function main_unbox_collection() :mixed{
var_dump(foo('foo'));
}
