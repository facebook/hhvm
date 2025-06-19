<?hh
final class MyValue {}

type MyShape = shape('value' => MyValue);

interface MySettings {
  abstract const type TWriteValue as mixed;
}

final class MyObj<T as MySettings> {}

final class MyGenericTest {

  public function get<T>(classname<T> $_class): T {
    throw new Exception("A");
  }

  public function set<
    T,
    TSettings as MySettings with { type TWriteValue super T },
  >(MyObj<TSettings> $_obj, T $_value): void {}

  public function set2<
    T,
    TSettings as MySettings with { type TWriteValue = T },
  >(MyObj<TSettings> $_obj, T $_value): void {}

  public function test(
    MyObj<MySettings with { type TWriteValue = dict<string, MyShape> }> $obj,
    MyObj<MySettings with { type TWriteValue = MyShape }> $obj2,
    MyObj<MySettings with { type TWriteValue = vec<MyShape> }> $obj3,
  ): void {
    // No Hack error
    $this->set($obj, dict[
      'key' => shape('value' => new MyValue()),
    ]);
    // Unexpected Hack error:
    $this->set($obj, dict[
      'key' => shape('value' => $this->get(MyValue::class)),
    ]);
    // No Hack error
    $this->set2($obj, dict[
      'key' => shape('value' => $this->get(MyValue::class)),
    ]);

    // No Hack error
    $this->set($obj2, shape('value' => new MyValue()));
    // No Hack error
    $this->set($obj2, shape('value' => $this->get(MyValue::class)));
    // No Hack error
    $this->set2($obj2, shape('value' => $this->get(MyValue::class)));

    $this->set($obj3, vec[
      shape('value' => new MyValue()),
    ]);
    // Unexpected Hack error:
    $this->set($obj3, vec[
      shape('value' => $this->get(MyValue::class)),
    ]);
    // No Hack error
    $this->set2($obj3, vec[
      shape('value' => $this->get(MyValue::class)),
    ]);
  }
}
