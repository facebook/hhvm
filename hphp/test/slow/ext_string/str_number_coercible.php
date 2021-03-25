<?hh

const GOOD_VALS = vec[
  '1',
  '-1',
  '1e2',
  ' 1',
  '2974394749328742328432',
  '-1e-2',
  '1',
  '-1',
  '1e2',
  ' 1',
  '2974394749328742328432',
  '-1e-2',
  '0xff',
  '0xff',
  '0123',
  '0123',
  '-0123',
  '+0123',
  '-0123',
  '+0123',
  '2018-08-01',
  '-0x80001',
  '+0x80001',
  '-0x80001.5',
  '0x80001.5',
  '0b0001010',
  '1 ',
  '1.2.4',
  '1e7.6',
  '3FF',
  '20 test',
  '3.6test',
  '1,000',
  '12345xxx',
  '12345 xxx',
  '12345.6xxx',
  '12345.6 xxx',
  '1_234_567',
];

const BAD_VALS = vec[
  '',
  'b12',
  '- 1',
  'NULL',
  'true',
];

<<__EntryPoint>> function main(): void {
  foreach (GOOD_VALS as $val) {
    invariant(HH\str_number_coercible($val), '%s should be coercible', $val);
  }
  foreach (BAD_VALS as $val) {
    invariant(!HH\str_number_coercible($val), '%s shouldn\'t be coercible', $val);
  }
  echo "All passed!\n";

}
