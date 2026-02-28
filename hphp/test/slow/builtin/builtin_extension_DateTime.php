<?hh

class A_DateTime extends DateTime {
  public $___x = 0;
  public function __clone() :mixed{
    $this->___x++;
  }
}

function test($cls, $args = vec[]) :mixed{
  echo $cls . "\n";
  $a = (new ReflectionClass($cls))->newInstanceArgs($args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && (get_class($c) !== null)) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));

  echo "================\n";

  $cls = 'A_' . $cls;
  echo $cls . "\n";
  $a = (new ReflectionClass($cls))->newInstanceArgs($args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && (get_class($c) !== null)) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));
}

function main() :mixed{
  echo "================\n";

  $y = new A_DateTime("2012-06-23T11:00:00");
  $y->___y = 73;
  $y2 = clone $y;
  $y2->___y++;
  $y2->modify("+3 days");

  var_dump($y);
  var_dump($y->format('Y-m-d'));
  var_dump($y2);
  var_dump($y2->format('Y-m-d'));
}

function DataTimeFromString() :mixed{
  echo "================\n";
  $num_tests = 10;
  $count_fill_error = 0;
  $count_nofill_error = 0;
  for ($i = 0; $i < $num_tests; $i++) {
    date_default_timezone_set('UTC');
    $curr_str = date('H:i:s');
    $format = 'Y-m-d';
    $rand_date = sprintf('%d-%02d-%02d',
                         rand(1900,2100),
                         rand(1,12),
                         rand(1,28));
    $date = DateTime::createFromFormat($format, $rand_date)
      ->format('H:i:s');
    if ($date != $curr_str) {
      // passing the above condition may be the result of change in
      // the system time. assuming running on a reasonable fast CPU
      // time cannot change on the second sample
      $curr_str = date('H:i:s');
      if ($date != $curr_str) {
        $count_fill_error++;
      }
    }
    if ('00:00:00' == $curr_str) {
      // just in case make sure the current time is not exact midnight!
      sleep(1);
      $curr_str = date('H:i:s');
    }
    // adding ! to the time format force to fill missing parts by
    // parts of Unix epoch is 1970-01-01 00:00:00 UTC
    $format = '!'.$format;
    $date = DateTime::createFromFormat($format, $rand_date)
      ->format('H:i:s');
    if ('00:00:00' != $date) {
      $count_no_fill_error++;
    }
  }
  if ($num_tests == $count_fill_error) {
    echo 'Error in createFromFormat when filling option is ON\n';
    echo $count_fill_error;
  }

  if ($num_tests == $count_nofill_error){
    echo 'Error in createFromFormat when filling option is OFF\n';
  }
}
<<__EntryPoint>> function main_entry(): void {
date_default_timezone_set('America/Los_Angeles');

test("DateTime", vec["2012-06-23T11:00:00"]);

main();

DataTimeFromString();
}
