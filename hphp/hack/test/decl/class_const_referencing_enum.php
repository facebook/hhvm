<?hh

enum Words: string as string {
  FOO = 'foo';
  BAR = 'bar';
}

<<__ConsistentConstruct>>
abstract class TestClassConstReferencingEnum {
  const dict<Words, float> WORD_VALUES = dict[
    Words::FOO => 1.0,
    Words::BAR => 2.0,
  ];
}
