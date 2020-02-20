<?hh // strict

function foo(int $x): int {
  $f = $y ==> $x * $x * $y;
  var_dump($f(10));
  return $f(10);
}

function double_foo(int $x): void {
  $f = $y ==> $z ==> $x * $y * $z;
  var_dump(($f(10))(5));
}

class C {
  public function __construct(public int $z) { }
  public static function func(): void { echo "hi\n"; }
  public function foo(int $x): (function(int): int) {
    $r = 4;
    $f = $y ==> {
      self::func();
      static::func();
      return $x * $y * $r * $this->z;
    };
    return $f;
  }
}

function wait(): Awaitable<void> {
  /* HH_FIXME[2049] */
  /* HH_FIXME[4026] */
  return RescheduleWaitHandle::create(0, 0);
}

class Argh {
  public static async function afoo(int $x): Awaitable<void> {
    await wait();
    $a = varray[$x, $x+1, $x+2];
    $y = await gena_(array_map(async $y ==> $x * $x * $y + foo($x), $a));
    var_dump($y);
  }
}

function prep<T>(Awaitable<T> $aw): T {
  return HH\Asio\join($aw);
}
async function gena_<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<array<Tv>> {
  await AwaitAllWaitHandle::fromArray($awaitables);
  return array_map($wh ==> \HH\Asio\result($wh), $awaitables);
}

async function lurr(): Awaitable<void> {
  $i = 12;
  await gena_(varray[Argh::afoo(100),
                    async {
                      await wait();
                      return await Argh::afoo($i);
                    }
                   ]);
}

final class C2 {
  public static function thing(Vector<string> $v): Set<string> {
    $domains = Set {};
    $v->map($line ==> {
      $d = $line . "!";
      /* HH_FIXME[4053] */
      $domains->add($d);
    });
    return $domains;
  }
}

function tparams<T>(): void {
  $f = $x ==> (T $y) ==> $y;
}

function test(): void {
  foo(10);
  double_foo(20);
  $f = (new C(1000))->foo(20);
  var_dump($f(3));

  prep(lurr());

  var_dump(C2::thing(Vector {"wut", "lol"}));
}
