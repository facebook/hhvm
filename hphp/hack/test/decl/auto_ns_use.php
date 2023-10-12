<?hh // strict

namespace {
  namespace HH\Lib\Dict {
    type MixedDict = dict<arraykey, mixed>;
  }

  function make_dict(): Dict\MixedDict {
    return dict[];
  }

  function get_value(Dict\MixedDict $d, arraykey $k): mixed {
    return $d[$k];
  }

  type MyDict = Dict\MixedDict;
}
