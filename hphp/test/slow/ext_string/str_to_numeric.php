<?hh

const GOOD_VALS = vec[
  "1",
  "-1",
  "1e2",
  " 1",
  "2974394749328742328432",
  "-1e-2",
  '1',
  '-1',
  '1e2',
  ' 1',
  '2974394749328742328432',
  '-1e-2',
  "0xff",
  '0xff',
  "0123",
  '0123',
  "-0123",
  "+0123",
  '-0123',
  '+0123',
];

const BAD_VALS = vec[
  "-0x80001",
  "+0x80001",
  "-0x80001.5",
  "0x80001.5",
  "",
  "1 ",
  "- 1",
  "1.2.4",
  "1e7.6",
  "3FF",
  "20 test",
  "3.6test",
  "1,000",
  "NULL",
  "true",
  "12345xxx",
  "12345 xxx",
  "12345.6xxx",
  "12345.6 xxx",
];

<<__EntryPoint>> function main(): void {
  foreach (GOOD_VALS as $val) {
    echo "successfully converted $val\n";
  }
  foreach (BAD_VALS as $val) {
    $v = HH\str_to_numeric($val);
    if ($v is nonnull) {
      echo "incorrectly converted $val to ".(string)$v."\n";
    } else {
      echo "successfully failed to convert $val\n";
    }
  }

}
