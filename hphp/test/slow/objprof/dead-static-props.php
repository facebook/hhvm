<?hh

function gen() {

  sleep(1);
  $sprop = '$sprop_'.time() % 60;
  $f = fopen($GLOBALS['filename'], "w");
  fwrite($f, <<<"EOD"
<?hh
class C {
  public static $sprop = "hello";
  function f() { return C::$sprop; }
}
EOD
  );
  fclose($f);
}

function visit_root($node) {
  if (!isset($node['type'])) return;
  if ($node['type'] === 'HPHP::StaticPropData') {
    echo $node['type'].' ';
    if (isset($node['class'])) {
      echo $node['class'].'::'.$node['prop'];
    } else {
      echo 'dead';
    }
    echo "\n";
  }
}

echo "start\n";
$filename = '/tmp/dead-static-props.php';
gen();
include $filename;
$c = new C;
$c->f(); // access static prop
$hg = heapgraph_create();
HH\heapgraph_foreach_root_node($hg, 'visit_root');
unlink($filename);
