<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

function testit(): void {
  // be careful, this will create a dict<Tatom(my_message), string>
  // so if you want to store multiple keys, you have to make sure
  // you're dictionary has type dict<string, ...> before adding an
  // atom in it.
  $dict = dict[];
  $dict[:@my_message] = "I am an atom !";

  foreach ($dict as $key => $value) {
    echo "key: ".$key."\n";
    echo "value: ".$value."\n";
  }

  $shape = shape("toto" => 42);
  echo $shape[:@toto]."\n";
}

class C {
  enum E {
    case int val;
  }
}

function gen(): dict<string, mixed> {
  return dict[];
}

function testit2<TP as C:@E>(TP $param): void {
  $data = gen();
  $_ = $data[$param];
}

type D = dict<C:@E, string>;

function testi3(D $dict, C:@E $key): ?string {
  return $dict[$key] ?? null;
}
