<?hh

function foo($feature1) {
  $feats = Vector{$feature1};
  sort($feats);
  return $feats[0];
}

var_dump(foo('foo'));
