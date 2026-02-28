<?hh

trait T1 {
  public int $f1 = 1;
}

trait T2 {
  public int $f2 = 2;
}

class C1 {
  public int $f3 = 3;
}

class C2 extends C1 {
  use T1;
  public int $f4 = 4;

  use T2;
  public int $f5 = 5;
}

class C3 extends C1 {
  use T1;
  // cannot redefine the field with different value
  // TODO should we check if the type should be matched?
  public string $f1 = 1;
}

class C4 {
  private int $f6 = 1;
}

class C5 extends C4 {
  private int $f6 = 1;
}

function test($obj) :mixed{
  $serialized = serialize($obj);
  print $serialized;
  print "\n";

  $deserialized = unserialize($serialized);
  var_dump($deserialized);
}

<<__EntryPoint>>
function main() :mixed{
  print "--- C1 ---\n";
  test(new C1());
  print "--- C2 ---\n";
  test(new C2());

  print "--- C4 ---\n";
  test(new C4());
  print "--- C5 ---\n";
  test(new C5());

  print "--- C3 ---\n";
  test(new C3());
}
