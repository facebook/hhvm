<?hh

class A {
  <<__Memoize>>
  public static async function outer($i) {
    if ($i == 0) return '';
    return await A::inner($i);
  }

  public static async function inner($i) {
    return 'number '.$i;
  }
}

async function submain() {
  // does eager return
  $z = await A::outer(20);
  concurrent {
    // no eager return inside a concurrent block
    $x = await A::outer(10);
    // should not share the same RDS handle with the previous calls
    $y = await A::outer(20);
  }
  var_dump($x.' '.$y.' '.$z);
}

<<__EntryPoint>>
async function main() {
  await submain();
  await submain();
}
