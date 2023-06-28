<?hh

function always_true() :mixed{
  return mt_rand(1, 2) < 10;
}

function noinline($x) :mixed{
  if (always_true()) {
    return $x;
  } else {
    return null;
  }
}

function exn($e) :mixed{
  echo get_class($e), ': ', $e->getMessage(), "\n";
}

function run_tests() :mixed{
  try { 1 % 0; } catch (\Throwable $e) { exn($e); }
  try { 1 % 0.0; } catch (\Throwable $e) { exn($e); }
  try { 1 % noinline(0); } catch (\Throwable $e) { exn($e); }
  try { 1 % noinline(0.0); } catch (\Throwable $e) { exn($e); }
  try { noinline(1) % 0; } catch (\Throwable $e) { exn($e); }
  try { noinline(1) % 0.0; } catch (\Throwable $e) { exn($e); }
  try { noinline(1) % noinline(0); }
    catch (\Throwable $e) { exn($e); }
  try { noinline(1) % noinline(0.0); }
    catch (\Throwable $e) { exn($e); }
}
<<__EntryPoint>>
function entrypoint_mod(): void {

  run_tests();
}
