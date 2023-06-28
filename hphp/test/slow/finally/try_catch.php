<?hh

function thr($doit) :mixed{
  try {
    var_dump("try");
    if ($doit) {
      throw new Exception('done');
    }
  } catch (Exception $e) {
    var_dump("catch");
  } finally {
    var_dump("finally");
  }
}

<<__EntryPoint>>
function main_try_catch() :mixed{
thr(true);
thr(false);
}
