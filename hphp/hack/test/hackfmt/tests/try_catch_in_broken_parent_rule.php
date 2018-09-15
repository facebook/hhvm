<?hh

doThing(
  $_ ==> {
    try {
      foo();
    } catch (Exception $e) {
      handleException($e);
    }
  },
  $some_other_argument,
  $some_third_argument,
);
