<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function  singleton<Tk  as supportdyn<mixed> as arraykey, Tv as supportdyn<mixed> >(~Tk $k, ~Tv $v): ~dict<Tk, Tv> {
  return dict[$k => $v];
}
