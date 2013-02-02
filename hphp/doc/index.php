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
      'Debugger' => array(
        'Getting Started' => 'debugger.start',
        'All Commands' => 'debugger.cmds',
        'Command A-Z' => 'debugger.refs',
      ),
      'HTTP Server' => array(
        'Rewrite Rules' => 'server.rewrite_rules',
        'SSL Setup' => 'server.ssl',
      ),
      'Inconsistencies' => array(
        'PHP vs. HipHop' => 'inconsistencies',
        'HipHop Compiler vs. Interpreter' => 'inconsistencies.hphpi',
      ),
      'New Features' => array(
        'New functions' => 'extension.new_functions',
        'yield and generator' => 'extension.yield',
        'Richer type hints' => 'extension.type_hints',
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
        'Server' => 'debug.server',
        'CPU Profiling' => 'debug.profile',
        'Memory leak detection' => 'debug.leak',
        'Mutex contention' => 'debug.mutex',
        'Useful gdb commands' => 'debug.gdb',
        'Useful Linux commands' => 'debug.linux',
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
    );

///////////////////////////////////////////////////////////////////////////////
// main

echo "<link type='text/css' rel='stylesheet' href='style.css' />";

$file = 'coding_guideline';
if (isset($_GET['file'])) $file = $_GET['file'];
$title = find_topics($topics, $file);

echo '<title>'.htmlspecialchars($title ? $title : 'Invalid topic').'</title>';
echo '<table cellpadding=0 cellspacing=0 border=0>';
echo '<tr><td valign=top width=200>';
format_index($topics, $file);
echo '</td><td valign=top bgcolor=white width=640>';
if (!$title) {
  echo 'Topic does not exist.';
} else {
  $doc = file_get_contents(realpath(dirname(__FILE__))."/$file");
  if (preg_match('/^debugger\./', $file)) {
    echo format_debugger_doc($doc);
  } else {
    echo format_document($doc);
  }
}
echo '</td></tr></table>';

///////////////////////////////////////////////////////////////////////////////
// helpers

function find_topics(&$topics, $file) {
  $title = '';

  $found_files = array();
  exec('find . -type f', $found_files);

  $files = array();
  foreach ($found_files as $f) {
    $f = substr($f, 2); // skipping "./"
    if (!preg_match('/(~|Makefile|index\.php|style\.css|www\.pid)/', $f)) {
      if ($f == $file) {
        $title = $f;
      }
      $files[$f] = $f;
    }
  }

  $allowed = isset($files[$file]);

  foreach ($topics as $topic => $group) {
    foreach ($group as $name => $f) {
      if ($f == $file) {
        $title = "$topic: $name";
      }
      unset($files[$f]);
    }
  }
  if (!empty($files)) {
    $topics['New Topics'] = $files;
  }

  return $title;
}

function format_index($topics, $file) {
  echo '<table cellpadding=1 cellspacing=3 border=1 bgcolor=white>';
  echo '<tr><td colspan=2 class="hphp">HipHopDoc</td></tr>';
  echo '<tr><td colspan=2>&nbsp;</td></tr>';
  foreach ($topics as $topic => $group) {
    echo "<tr><td colspan=2 class='topic'>";
    echo "<nobr>$topic</nobr></a></td></tr>";

    foreach ($group as $name => $f) {
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
  $doc = preg_replace('/<(?!\/?(b|h2|i)[ >])/',
                      '&lt;', $doc);               // unsupported tags
  $doc = preg_replace('/\n= (.*?)\n/',
                      "<h3>\\1</h3>\n", $doc);     // h3 headers
  $doc = preg_replace('/\n([0-9]+\. )(.*?)\n/',
                      "<h3>\\1\\2</h3>\n", $doc);  // 1. 2. 3.
  $doc = preg_replace('/\n(\([0-9]+\) )(.*?)\n/',
                      "\n\\1<u>\\2</u>\n", $doc);  // (1) (2) (3)
  $doc = preg_replace('/((?:\n- [^\n]*(?:\n  [^\n]+)*)+)/',
                      "\n<ul>\\1</ul>\n", $doc);   // lists
  $doc = preg_replace('/\n- /', "\n<li>", $doc);   // list items
  $doc = preg_replace('/((?:\n(  |\t)[^\n]*)+)/',
                      "\n<pre>\\1</pre>\n", $doc); // code blocks
  $doc = preg_replace('/(<li>[^\n]*)\n<pre>(.*?)<\/pre>/s',
                      "\\1 \\2", $doc);            // fix list item's 2nd lines

  $doc = preg_replace('/\n([^ \t]+): (.*)/',
                      "<br><b class=item_header>\\1</b>: ".
                      "<span class=item_details>\\2</span>",
                      $doc);                       // item: details

  $doc = preg_replace('/\n\n/', '<p>', $doc);      // paragraphs
  $doc = preg_replace('/\[\[[ \n]*(.*?)[ \n]*\|[ \n]*(.*?)[ \n]*\]\]/s',
                      '<a href="\\1">\\2</a>',$doc); // links

  // copyright notice
  $doc .= '<p>&nbsp;<table width="100%"><tr><td class="footer" align=right>'.
    'Facebook &copy; 2009</td></tr></table>';
  return $doc;
}

function format_debugger_doc($doc) {
  $doc = preg_replace('/ *(?:\xe2\x94\x80|\-){5,}(.*) '.
                      '(?:\xe2\x94\x80|\-){5,}/',
                      '<h2>$1</h2>', $doc);
  $doc = preg_replace("/('\[.*?')/", '<b>$1</b>', $doc);
  $doc = preg_replace("/(\{.*?\})/", '<i>$1</i>', $doc);
  return format_document($doc);
}
