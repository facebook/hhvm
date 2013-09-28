<?php
foreach (array(2440588, 2452162, 2453926) as $jd) {
  echo "### JD $jd ###\n";
  for ($mode = 0; $mode <= 6; $mode++) {
    echo "--- mode $mode ---\n";
    for ($offset = 0; $offset <= 12; $offset++) {
      echo jdmonthname($jd + $offset * 30, $mode). "\n";
    }
  }
}
?>