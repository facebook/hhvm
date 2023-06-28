<?hh

class C { function m() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $mc = meth_caller(C::class, 'm');
  $lv = __hhvm_intrinsics\launder_value($mc);

  try { var_dump(serialize($mc)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(serialize($lv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(fb_serialize($mc)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(fb_serialize($lv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(fb_compact_serialize($mc)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(fb_compact_serialize($lv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(json_encode($mc)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(json_encode($lv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  $mcv = vec[$mc];
  $lvv = __hhvm_intrinsics\launder_value(vec[$lv]);

  try { var_dump(serialize($mcv)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(serialize($lvv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(fb_serialize($mcv)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(fb_serialize($lvv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(fb_compact_serialize($mcv)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(fb_compact_serialize($lvv)); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(json_encode($mcv)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(json_encode($lvv)); } catch (Exception $e) { var_dump($e->getMessage()); }
}

