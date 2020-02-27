<?hh

enum E0 : int { };
enum E1 : int { C0 = 0; };
enum E2 : int { C0 = 0; C1 = 1; };
enum E3 : int { C0 = 0; C1 = 1; C2 = 2; };
enum E4 : int { C0 = 0; C1 = 1; C2 = 2; C3 = 3; };

<<__EntryPoint>>
function main() {
  foreach (vec[E0::class, E1::class, E2::class, E3::class, E4::class] as $cls) {
    print("\nTesting ".$cls.":\n");
    print('names = '.json_encode($cls::getNames())."\n");
    print('values = '.json_encode($cls::getValues())."\n");
    print('names.tag = '.HH\get_provenance($cls::getNames())."\n");
    print('values.tag = '.HH\get_provenance($cls::getValues())."\n");
  }

  print("\nTesting static E2:\n");
  print('names.tag = '.HH\get_provenance(E2::getNames())."\n");
  print('values.tag = '.HH\get_provenance(E2::getValues())."\n");

  print("\nTesting static E4:\n");
  print('names.tag = '.HH\get_provenance(E4::getNames())."\n");
  print('values.tag = '.HH\get_provenance(E4::getValues())."\n");
}
