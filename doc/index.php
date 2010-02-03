<?php

$css = 'style'; // default
if (isset($_GET['css'])) $css = $_GET['css'];
echo "<link type='text/css' rel='stylesheet' href='$css.css' />";

$file = 'debug.leak';
if (isset($_GET['file'])) $file = $_GET['file'];
$doc = file_get_contents(realpath(dirname(__FILE__))."/$file");

echo '<table cellpadding=0 cellspacing=0 border=0 bgcolor="#3B5998">';
echo '<tr><td valign=top width=200>';
echo format_index($file);
echo '</td><td valign=top bgcolor=white width=640>';
echo format_document($doc);
echo '</td></tr></table>';

///////////////////////////////////////////////////////////////////////////////
// helpers

function format_index($file) {
  $files = array();
  exec('ls '. realpath(dirname(__FILE__)), $files);
  echo '<table cellpadding=4 cellspacing=4 border=0>';
  echo '<tr><td class="hphp">HipHop</td></tr>';
  echo '<tr><td>&nbsp;</td></tr>';
  foreach ($files as $f) {
    if (preg_match('/(~|Makefile|index\.php|style\.css|www\.pid)/', $f)) {
      continue;
    }
    echo '<tr><td>';
    if (is_dir(realpath(dirname(__FILE__)).'/'.$f)) {
      echo "<span class='$class'><a href='$f/index.php'>$f</a></span>";
    } else {
      $class = $f == $file ? 'current_file' : 'file';
      echo "<span class='$class'><a href='index.php?file=".
        urlencode($f)."'>$f</a></span>";
      echo '</td></tr>';
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
