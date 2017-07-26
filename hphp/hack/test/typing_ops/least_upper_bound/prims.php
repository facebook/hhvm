<?hh

  function test () {
    new least_upper_bound<int, int, int>();
    new least_upper_bound<int, string>();
    new least_upper_bound<float, int>();
    new least_upper_bound<float, num>();
    new least_upper_bound<bool, void>();
    new least_upper_bound<void>();
    new least_upper_bound<float, int, num>();
    new least_upper_bound<float, num, int>();
  }
