<?php
$topics =
    array(
      'Compiler' => array(
        'Command line options' => 'command.compiler',
        'Configurable options' => 'options.compiler',
      ),
      'Compiled Program' => array(
        'Command line options' => 'command.compiled',
        'Configurable options' => 'options.compiled',
        'Administrative commands' => 'command.admin_server',
        'Server Status' => 'server.status',
        'Server Statistics' => 'server.stats',
      ),
      'PHP Compatibility' => array(
        'Inconsistencies' => 'inconsistencies',
      ),
      'New Features' => array(
        'New functions' => 'extension.new_functions',
        'Parallel execution' => 'threading',
        'Server documents' => 'server.documents',
        'RPC server' => 'server.rpc_server',
        'Xbox server' => 'server.xbox_server',
        'Dangling server' => 'server.dangling_server',
      ),
      'Foreign Function Interfaces' => array(
        'C++' => 'ffi.c++',
        'Java' => 'ffi.java',
        'Python' => 'ffi.python',
      ),
      'Debugging' => array(
        'CPU Profiling' => 'debug.profile',
        'Memory leak detection' => 'debug.leak',
        'Mutex contention' => 'debug.mutex',
        'Useful gdb commands' => 'debug.gdb',
      ),
      'Development' => array(
        'Coding guidelines' => 'coding_guideline',
        'Type system' => 'runtime.type_system',
        'Memory model' => 'runtime.memory_model',
        'Writing new extensions' => 'extension.development',
        'Useful commands' => 'command.project',
      ),
      'References' => array(
        'Configuration file format' => 'hdf',
      ),
      'Facebook Build Processes' => array(
        'Compiler push' => 'facebook/fb.process.merge',
        'Server testing' => 'facebook/fb.process.server_test',
        'External library' => 'facebook/fb.process.external_lib',
        'Update engshare' => 'facebook/fb.process.engshare',
        'Build worker machine' => 'facebook/fb.process.build_worker',
      ),
    );

///////////////////////////////////////////////////////////////////////////////
// main

$css = 'style'; // default
if (isset($_GET['css'])) $css = $_GET['css'];
echo "<link type='text/css' rel='stylesheet' href='$css.css' />";

$file = 'coding_guideline';
if (isset($_GET['file'])) $file = $_GET['file'];
$doc = file_get_contents(realpath(dirname(__FILE__))."/$file");

echo '<table cellpadding=0 cellspacing=0 border=0>';
echo '<tr><td valign=top width=200>';
echo format_index($file);
echo '</td><td valign=top bgcolor=white width=640>';
echo format_document($doc);
echo '</td></tr></table>';

///////////////////////////////////////////////////////////////////////////////
// helpers

function format_index($file) {
  global $topics;

  $found_files = array();
  exec('find . -type f', $found_files);

  $files = array();
  foreach ($found_files as $f) {
    $f = substr($f, 2); // skipping "./"
    if (!preg_match('/(~|Makefile|index\.php|style\.css|www\.pid)/', $f)) {
      $files[$f] = $f;
    }
  }

  foreach ($topics as $topic => $group) {
    foreach ($group as $name => $f) {
      unset($files[$f]);
    }
  }
  if (!empty($files)) {
    $topics['New Topics'] = $files;
  }

  echo '<table cellpadding=1 cellspacing=3 border=1 bgcolor=white>';
  echo '<tr><td colspan=2 class="hphp">HipHopDoc</td></tr>';
  echo '<tr><td colspan=2>&nbsp;</td></tr>';
  foreach ($topics as $topic => $group) {
    echo "<tr><td colspan=2 class='topic'>";
    echo "<nobr>$topic</nobr></a></td></tr>";

    foreach ($group as $name => $f) {
      unset($files[$f]);

      echo '<tr><td width=5></td><td><nobr>';
      $class = $f == $file ? 'current_file' : 'file';
      echo "<span class='$class'><a href='index.php?file=".
        urlencode($f)."'>$name</a></span>";
      echo '</nobr></td></tr>';
    }
  }
  echo '</table>';
}

function format_document($doc) {
  $doc = preg_replace('/\n= (.*?)\n/',
                      "<h3>\\1</h3>\n", $doc);     // h3 headers
  $doc = preg_replace('/\n([0-9]+\. )(.*?)\n/',
                      "<h3>\\1\\2</h3>\n", $doc);  // 1. 2. 3.
  $doc = preg_replace('/\n(\([0-9]+\) )(.*?)\n/',
                      "\n\\1<u>\\2</u>\n", $doc);  // (1) (2) (3)
  $doc = preg_replace('/((?:\n- [^\n]*(?:\n  [^\n]+)*)+)/',
                      "\n<ul>\\1</ul>\n", $doc);   // lists
  $doc = preg_replace('/\n- /', "\n<li>", $doc);   // list items
  $doc = preg_replace('/((?:\n  [^\n]*)+)/',
                      "\n<pre>\\1</pre>\n", $doc); // code blocks
  $doc = preg_replace('/(<li>[^\n]*)\n<pre>(.*?)<\/pre>/s',
                      "\\1 \\2", $doc);            // fix list item's 2nd lines

  $doc = preg_replace('/\n([^ \t]+): (.*)/',
                      "<br><b class=item_header>\\1</b>: ".
                      "<span class=item_details>\\2</span>",
                      $doc);                       // item: details

  $doc = preg_replace('/\n\n/', '<p>', $doc);      // paragraphs
  $doc = preg_replace('/<T>/', '&lt;T&gt;', $doc); // C++ templates

  // copyright notice
  $doc .= '<p>&nbsp;<table width="100%"><tr><td class="footer" align=right>'.
    'Facebook &copy; 2009</td></tr></table>';
  return $doc;
}
