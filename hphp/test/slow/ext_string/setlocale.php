<?php

$cwd = getcwd();
$file_path = __FILE__;
$relative_file_path = str_replace($cwd, '', $file_path);

$is_pagelet = !empty($_GET) && ($_GET['pagelet'] == 'true');

if (!$is_pagelet) {
  // Let's try some terrible locales to ensure they fail
  var_dump(setlocale(LC_ALL, 'LC_ALL=it_IT'));
  var_dump(setlocale(LC_ALL, 'LC_ALL=it_IT;'));
  var_dump(setlocale(LC_CTYPE, 'LC_CTYPE=it_IT'));
  var_dump(setlocale(LC_CTYPE, 'LC_CTYPE=it_IT;'));
  var_dump(setlocale(LC_ALL, 'LC_CTYPE=it_IT'));
  var_dump(setlocale(LC_ALL, 'LC_CTYPE=it_IT;'));

  // And now a good one that will work
  setlocale(LC_TIME, 'it_IT');
  setlocale(LC_NUMERIC, 'it_IT');
} else {
  if (!empty($_GET['locale'])) {
    setlocale(LC_TIME, $_GET['locale']);
  }
}

// In non-pagelet mode, expecting Italian date and decimal point format: 3,142 and venerdi 22 dicembre 1978.
// In pagelet mode expecting regular decimal point(.) and Dutch (or default) date (vrijdag 22 december 1978) depending on what's passed in.
echo ($is_pagelet ? "Pagelet: " : "Not pagelet: ") . sprintf("%.3f: ", 3.142) . strftime("%A %e %B %Y\n", mktime(0, 0, 0, 12, 22, 1978));

if (!$is_pagelet) {
  if (!pagelet_server_is_enabled()) {
    echo "This test needs pagelet server enabled\n";
    die();
  }

  // Send to pagelet multiple times to ensure the previously set locale does not linger into the next request.
  // We rely on the opt PageletServer.ThreadCount=1 to ensure the same thread gets the request.
  for ($i = 0; $i < 10; $i++) {
    $locale = 'nl_NL';
    if ($i % 2 == 0) {
      $locale = '';
    }

    send_to_pagelet($relative_file_path, $locale);
  }

  // Echo it again. If setlocale is global, the pagelet thread will have changed this output line to Dutch
  echo strftime("%A %e %B %Y\n", mktime(0, 0, 0, 12, 22, 1978));
}

function send_to_pagelet($relative_file_path, $locale) {
  $headers = array();
  $task = pagelet_server_task_start("$relative_file_path/?pagelet=true&locale=$locale", $headers, 'dummy');
  if (is_null($task)) {
    echo "Failed to start pagelet task\n";
    die();
  }

  $rc = 0;
  $result = pagelet_server_task_result($task, $headers, $rc, 2000);
  if ($rc != 200) {
    echo "Failed to finish pagelet task\n";
    die();
  }

  echo trim($result) . "\n";
}
