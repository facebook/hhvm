<?hh

class W<reify T> {}
class R<reify T> {}
class Plain {}

// Hack array hints: the reified argument is never checked at runtime.
function p_vec<reify T>(vec<T> $x): void {}
function p_dict<reify T>(dict<string, T> $x): void {}
function p_keyset<reify T as arraykey>(keyset<T> $x): void {}
function p_varray<reify T>(varray<T> $x): void {}
function p_darray<reify T>(darray<string, T> $x): void {}
function p_vec_or_dict<reify T>(vec_or_dict<T> $x): void {}
function p_varray_or_darray<reify T>(varray_or_darray<T> $x): void {}
function p_any_array<reify T>(AnyArray<string, T> $x): void {}
function p_nullable_vec<reify T>(?vec<T> $x): void {}
function p_vec_of_w<reify T>(vec<W<T>> $x): void {}
function p_soft_vec<reify T>(<<__Soft>> vec<T> $x): void {}
function r_vec<reify T>(mixed $x): vec<T> { return $x; }
async function r_async_vec<reify T>(mixed $x): Awaitable<vec<T>> { return $x; }

// Class hints: the reified argument is checked, so mismatches must still throw.
function p_bare<reify T>(T $x): void {}
function p_w<reify T>(W<T> $x): void {}
function p_w_vec<reify T>(W<vec<T>> $x): void {}
function p_w_nullable_vec<reify T>(W<?vec<T>> $x): void {}
function r_w_vec<reify T>(mixed $x): W<vec<T>> { return $x; }

async function try_it(
  string $label,
  (function(): Awaitable<void>) $f,
): Awaitable<void> {
  echo $label."\n";
  try {
    await $f();
    echo "  ok\n";
  } catch (Exception $e) {
    echo "  EXN: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  set_error_handler(
    (int $errno, string $str, ...$rest) ==> {
      if ($errno === E_RECOVERABLE_ERROR) {
        throw new Exception($str);
      }
      echo "  WARN: ".$str."\n";
      return true;
    },
  );

  await try_it('vec<int> <- vec[1,2]', async () ==> { p_vec<int>(vec[1, 2]); });
  await try_it('vec<string> <- vec[1,2]', async () ==> { p_vec<string>(vec[1, 2]); });

  await try_it('dict<string,int> <- dict', async () ==> { p_dict<int>(dict['a' => 1]); });
  await try_it('dict<string,string> <- dict', async () ==> { p_dict<string>(dict['a' => 1]); });

  await try_it('keyset<int> <- keyset', async () ==> { p_keyset<int>(keyset[1, 2]); });
  await try_it('keyset<string> <- keyset', async () ==> { p_keyset<string>(keyset[1, 2]); });

  await try_it('varray<string> <- vec[1,2]', async () ==> { p_varray<string>(varray[1, 2]); });
  await try_it('darray<string,string> <- darray', async () ==> { p_darray<string>(darray['a' => 1]); });
  await try_it('vec_or_dict<string> <- vec[1,2]', async () ==> { p_vec_or_dict<string>(vec[1, 2]); });
  await try_it('varray_or_darray<string> <- varray', async () ==> { p_varray_or_darray<string>(varray[1, 2]); });
  await try_it('AnyArray<string,string> <- dict', async () ==> { p_any_array<string>(dict['a' => 1]); });

  await try_it('?vec<string> <- null', async () ==> { p_nullable_vec<string>(null); });
  await try_it('?vec<string> <- vec[1,2]', async () ==> { p_nullable_vec<string>(vec[1, 2]); });

  await try_it('vec<string> <- new W<int>', async () ==> { p_vec<string>(new W<int>()); });
  await try_it('vec<string> <- new Plain', async () ==> { p_vec<string>(new Plain()); });
  await try_it('vec<string> <- dict', async () ==> { p_vec<string>(dict['a' => 1]); });
  await try_it('vec<string> <- 1', async () ==> { p_vec<string>(1); });
  await try_it('vec<string> <- null', async () ==> { p_vec<string>(null); });
  await try_it('?vec<string> <- new W<int>', async () ==> { p_nullable_vec<string>(new W<int>()); });
  await try_it('return vec<string> <- new W<int>', async () ==> { r_vec<string>(new W<int>()); });

  await try_it('vec<W<int>> <- vec[new W<int>]', async () ==> { p_vec_of_w<int>(vec[new W<int>()]); });
  await try_it('vec<W<int>> <- vec[new W<string>]', async () ==> { p_vec_of_w<int>(vec[new W<string>()]); });

  await try_it('@vec<string> <- vec[1,2]', async () ==> { p_soft_vec<string>(vec[1, 2]); });
  await try_it('@vec<string> <- new R<int>', async () ==> { p_soft_vec<string>(new R<int>()); });
  await try_it('@vec<string> <- new Plain', async () ==> { p_soft_vec<string>(new Plain()); });
  await try_it('@vec<string> <- dict', async () ==> { p_soft_vec<string>(dict['a' => 1]); });
  await try_it('@vec<string> <- 1', async () ==> { p_soft_vec<string>(1); });

  await try_it('return vec<int> <- vec[1,2]', async () ==> { r_vec<int>(vec[1, 2]); });
  await try_it('return vec<string> <- vec[1,2]', async () ==> { r_vec<string>(vec[1, 2]); });

  await try_it('return Awaitable<vec<string>> <- vec[1,2]', async () ==> { await r_async_vec<string>(vec[1, 2]); });

  await try_it('T=int <- 1', async () ==> { p_bare<int>(1); });
  await try_it('T=string <- 1', async () ==> { p_bare<string>(1); });

  await try_it('W<int> <- new W<int>', async () ==> { p_w<int>(new W<int>()); });
  await try_it('W<string> <- new W<int>', async () ==> { p_w<string>(new W<int>()); });

  await try_it('W<vec<int>> <- new W<vec<int>>', async () ==> { p_w_vec<int>(new W<vec<int>>()); });
  await try_it('W<vec<string>> <- new W<vec<int>>', async () ==> { p_w_vec<string>(new W<vec<int>>()); });

  await try_it('W<?vec<int>> <- new W<?vec<int>>', async () ==> { p_w_nullable_vec<int>(new W<?vec<int>>()); });
  await try_it('W<?vec<string>> <- new W<?vec<int>>', async () ==> { p_w_nullable_vec<string>(new W<?vec<int>>()); });

  await try_it('return W<vec<int>> <- new W<vec<int>>', async () ==> { r_w_vec<int>(new W<vec<int>>()); });
  await try_it('return W<vec<string>> <- new W<vec<int>>', async () ==> { r_w_vec<string>(new W<vec<int>>()); });
}
