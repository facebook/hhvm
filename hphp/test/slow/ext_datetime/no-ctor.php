<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MyDateTime extends DateTime {
  public function __construct() {}
}

class MyDateTimeZone extends DateTimeZone {
  public function __construct() {}
}

class MyDateInterval extends DateInterval {
  public function __construct() {}
}

<<__EntryPoint>> function test(): void {
  try {
    $x = new MyDateTime();
    $y = clone $x;
    var_dump($y->getOffset());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $x = new MyDateTimeZone();
    $y = clone $x;
    var_dump($y->getName());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $x = new MyDateInterval();
    $y = clone $x;
    var_dump($y->format(""));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
