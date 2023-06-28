<?hh

class C { function m() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $mc = meth_caller(C::class, 'm');
  $lv = __hhvm_intrinsics\launder_value($mc);

  var_dump(serialize($mc));
  var_dump(serialize($lv));

  var_dump(fb_serialize($mc));
  var_dump(fb_serialize($lv));

  var_dump(fb_compact_serialize($mc));
  var_dump(fb_compact_serialize($lv));

  var_dump(json_encode($mc));
  var_dump(json_encode($lv));

  $mcv = vec[$mc];
  $lvv = __hhvm_intrinsics\launder_value(vec[$lv]);

  var_dump(serialize($mcv));
  var_dump(serialize($lvv));

  var_dump(fb_serialize($mcv));
  var_dump(fb_serialize($lvv));

  var_dump(fb_compact_serialize($mcv));
  var_dump(fb_compact_serialize($lvv));

  var_dump(json_encode($mcv));
  var_dump(json_encode($lvv));
}
