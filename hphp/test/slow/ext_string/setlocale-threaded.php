<?hh

function send_to_pagelet($relative_file_path, $locale) :mixed{
  $headers = dict[];
  $task = pagelet_server_task_start(
    "$relative_file_path/?pagelet=true&locale=$locale", $headers, 'dummy'
  );
  if (is_null($task)) {
    echo "Failed to start pagelet task\n";
    exit();
  }

  $rc = 0;
  $result = pagelet_server_task_result($task, inout $headers, inout $rc, 2000);
  if ($rc != 200) {
    echo "Failed to finish pagelet task, status = $rc\n";
    exit();
  }

  echo trim($result) . "\n";
}

function escape_non_ascii($str) :mixed{
  $count = -1;
  return preg_replace_callback('/([\x00-\x1F\x7F-\xFF]+)/',
    function ($match) { return urlencode($match[1]); },
    $str, -1, inout $count);
}


<<__EntryPoint>>
function main_setlocale_threaded() :mixed{
$cwd = getcwd();
$file_path = __FILE__;
$relative_file_path = str_replace($cwd, '', $file_path);

$is_pagelet = ((bool)$_GET ?? false) && ($_GET['pagelet'] == 'true');

if (!$is_pagelet) {
  // And now a good one that will work
  setlocale(LC_TIME, 'fr_FR');
  setlocale(LC_NUMERIC, 'fr_FR');
} else {
  if ($_GET['locale'] ?? false) {
    setlocale(LC_TIME, $_GET['locale']);
  }
}

// In non-pagelet mode, expecting Italian date and decimal point format:
//   3,142 and venerdi 22 dicembre 1978.
// In pagelet mode expecting regular decimal point(.) and Dutch (default) date
//   (vrijdag 22 december 1978) depending on what's passed in.
echo ($is_pagelet ? "Pagelet: " : "Not pagelet: "),
     sprintf("%.3f: ", 3.142),
     escape_non_ascii(strftime("%A %e %B %Y", mktime(0, 0, 0, 12, 22, 1978))),
     "\n";

if (!$is_pagelet) {
  if (!pagelet_server_is_enabled()) {
    echo "This test needs pagelet server enabled\n";
    exit();
  }

  // Send to pagelet multiple times to ensure the previously set locale
  // does not linger into the next request.
  // We rely on the opt PageletServer.ThreadCount=1 to ensure the
  // same thread gets the request.
  for ($i = 0; $i < 10; $i++) {
    $locale = 'de_DE';
    if ($i % 2 == 0) {
      $locale = '';
    }

    send_to_pagelet($relative_file_path, $locale);
  }

  // Echo it again. If setlocale is global, the pagelet thread will have
  // changed this output line to Dutch
  echo escape_non_ascii(strftime("%A %e %B %Y", mktime(0, 0, 0, 12, 22, 1978)));
}
}
