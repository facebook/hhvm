<?hh
// T134628593
class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

class D extends C {
  const dict<string, mixed> DICT = dict['a' => 42];




}

class E extends D {



}

function main(): void {
    idx(E::DICT, 'b');
}
