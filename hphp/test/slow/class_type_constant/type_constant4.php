<?hh

interface I {
  abstract const type T;
  abstract const int type;
}
class C implements I {
  const type T = string;
  const type = 0;

  public static function not_enforced(self::T $x): self::T {
    return 0;
  }
}

function not_enforced(C::T $x): C::T {
  return 0;
}

async function gen_not_enforced(Awaitable<C::T> $x): Awaitable<C::T> {
  return 0;
}


<<__EntryPoint>>
function main_type_constant4() :mixed{
C::not_enforced(null);
not_enforced(null);

HH\Asio\join(gen_not_enforced(async { return null; }));
}
