<?hh

function check_mt_srand(): void {
  mt_srand(1);
  $expected = vec[mt_rand(), mt_rand(), mt_rand()];

  mt_srand(1);
  $got = vec[mt_rand(), mt_rand(), mt_rand()];

  print("srand sequences repeat: ".($expected == $got)."\n");
  print("srand sequences are variable: ".($got[0] != $got[1] || $got[0] != $got[2])."\n");
}

function check_mt_rand(): void {
  $num_trials = 1000;

  $initial_val = mt_rand();
  $total = $initial_val;
  $min = $initial_val;
  $max = $initial_val;

  for ($i = 1; $i < $num_trials; $i++) {
    $val = mt_rand();
    $total += $val;
    if ($val < $min) {
      $min = $val;
    }
    if ($val > $max) {
      $max = $val;
    }
  }

  // The probability that NONE of the values land in a given 10% interval are
  // (90%)^1000, or 1.75e-46.
  print("rand min is okay: ".($min < 0.1 * \HH\Lib\Math\INT32_MAX)."\n");
  print("rand max is okay: ".($max > 0.9 * \HH\Lib\Math\INT32_MAX)."\n");

  // The variance is (range^2 - 1)/count, or 2.306e+18. Using the central limit theorem,
  // the statistic sqrt(n) * (X - mu) has a distribution of Norm(0, 2.306e+18). The
  // 1e-12 quantile is -4360853306 and the 1 - 1e-12 quantile is 4360855218.
  $statistic = sqrt(floatval($num_trials)) * ($total/$num_trials - \HH\Lib\Math\INT32_MAX / 2);
  $mean_is_believable = ($statistic >= -4360853306 && $statistic <= 4360855218);
  if ($mean_is_believable) {
    print("Mean is believable.\n");
  } else {
    print("Mean is excessive -- mean: ".($total/$num_trials).", statistic: ".$statistic."\n");
  }
}

<<__EntryPoint>> function main(): void {
  check_mt_rand();
  check_mt_srand();
}
