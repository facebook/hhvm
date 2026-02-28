<?hh

function always_true() :mixed{
  return mt_rand(1, 2) < 10;
}

function id($x) :mixed{
  return $x;
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
  echo 3 << 64, "\n";
  echo 3 << id(64), "\n";
  echo 3 << noinline(64), "\n";
  echo id(3) << 64, "\n";
  echo id(3) << id(64), "\n";
  echo id(3) << noinline(64), "\n";
  echo noinline(3) << 64, "\n";
  echo noinline(3) << id(64), "\n";
  echo noinline(3) << noinline(64), "\n";

  echo 3 << 65, "\n";
  echo 3 << id(65), "\n";
  echo 3 << noinline(65), "\n";
  echo id(3) << 65, "\n";
  echo id(3) << id(65), "\n";
  echo id(3) << noinline(65), "\n";
  echo noinline(3) << 65, "\n";
  echo noinline(3) << id(65), "\n";
  echo noinline(3) << noinline(65), "\n";

  echo 3 >> 64, "\n";
  echo 3 >> id(64), "\n";
  echo 3 >> noinline(64), "\n";
  echo id(3) >> 64, "\n";
  echo id(3) >> id(64), "\n";
  echo id(3) >> noinline(64), "\n";
  echo noinline(3) >> 64, "\n";
  echo noinline(3) >> id(64), "\n";
  echo noinline(3) >> noinline(64), "\n";

  echo 3 >> 65, "\n";
  echo 3 >> id(65), "\n";
  echo 3 >> noinline(65), "\n";
  echo id(3) >> 65, "\n";
  echo id(3) >> id(65), "\n";
  echo id(3) >> noinline(65), "\n";
  echo noinline(3) >> 65, "\n";
  echo noinline(3) >> id(65), "\n";
  echo noinline(3) >> noinline(65), "\n";

  echo "-\n";

  echo -3 << 64, "\n";
  echo -3 << id(64), "\n";
  echo -3 << noinline(64), "\n";
  echo id(-3) << 64, "\n";
  echo id(-3) << id(64), "\n";
  echo id(-3) << noinline(64), "\n";
  echo noinline(-3) << 64, "\n";
  echo noinline(-3) << id(64), "\n";
  echo noinline(-3) << noinline(64), "\n";

  echo -3 << 65, "\n";
  echo -3 << id(65), "\n";
  echo -3 << noinline(65), "\n";
  echo id(-3) << 65, "\n";
  echo id(-3) << id(65), "\n";
  echo id(-3) << noinline(65), "\n";
  echo noinline(-3) << 65, "\n";
  echo noinline(-3) << id(65), "\n";
  echo noinline(-3) << noinline(65), "\n";

  echo -3 >> 64, "\n";
  echo -3 >> id(64), "\n";
  echo -3 >> noinline(64), "\n";
  echo id(-3) >> 64, "\n";
  echo id(-3) >> id(64), "\n";
  echo id(-3) >> noinline(64), "\n";
  echo noinline(-3) >> 64, "\n";
  echo noinline(-3) >> id(64), "\n";
  echo noinline(-3) >> noinline(64), "\n";

  echo -3 >> 65, "\n";
  echo -3 >> id(65), "\n";
  echo -3 >> noinline(65), "\n";
  echo id(-3) >> 65, "\n";
  echo id(-3) >> id(65), "\n";
  echo id(-3) >> noinline(65), "\n";
  echo noinline(-3) >> 65, "\n";
  echo noinline(-3) >> id(65), "\n";
  echo noinline(-3) >> noinline(65), "\n";

  echo "-\n";

  try {
    echo (new stdClass) >> 64, "\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    echo (new stdClass) << 64, "\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "-\n";

  try {echo 3 << -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo 3 << id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo 3 << noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) << -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) << id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) << noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) << -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) << id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) << noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}

  try {echo 3 >> -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo 3 >> id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo 3 >> noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) >> -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) >> id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo id(3) >> noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) >> -1, "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) >> id(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
  try {echo noinline(3) >> noinline(-1), "\n";}
    catch (\Throwable $e) {exn($e);}
}
<<__EntryPoint>>
function entrypoint_shift(): void {

  run_tests();
}
