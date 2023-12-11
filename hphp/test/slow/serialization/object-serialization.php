<?hh

class NoProps {}

class OneProp {
  public function __construct(
    public mixed $x = null,
  ) {}
}

<<__EntryPoint>>
function main() :mixed{
  $test_cases = vec[
    new NoProps(),
    new OneProp(),
    vec[new NoProps()],
    vec[new OneProp()],
    new OneProp(vec[new NoProps()]),
    vec[new OneProp(vec[new NoProps()])],
  ];
  foreach ($test_cases as $test_case) {
    print("============================================\n");
    // 6 is the type for internal serialization.
    $ser = __hhvm_intrinsics\serialize_with_format($test_case, 6);
    print($ser."\n");
    // 1 is the type for internal deserialization.
    $des = __hhvm_intrinsics\deserialize_with_format($ser, 1);
    $ser_match = $ser === __hhvm_intrinsics\serialize_with_format($des, 6);
    print("Objects ".($test_case == $des ? "match!\n" : "differ!\n"));
    print("Serializations ".($ser_match ? "match!\n" : "differ!\n"));
    print("\n");
  }
}
