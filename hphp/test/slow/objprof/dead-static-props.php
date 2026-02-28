<?hh

function gen() :mixed{

  sleep(1);
  $sprop = '$sprop_'.time() % 60;
  $f = fopen(\HH\global_get('filename'), "w");
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

function visit_root($node) :mixed{
  if (!isset($node['type'])) return;
  if ($node['type'] === 'HPHP::StaticPropData' ||
      $node['type'] === 'HPHP::StaticMultiPropData') {
    // TODO(kshaunak): Clean up and exclude dead static props correctly.
  }
}
<<__EntryPoint>>
function entrypoint_deadstaticprops(): void {

  echo "start\n";
  \HH\global_set('filename', sys_get_temp_dir().'/dead-static-props.php');
  gen();
  include \HH\global_get('filename');
  $c = new C;
  $c->f(); // access static prop
  $hg = heapgraph_create();
  HH\heapgraph_foreach_root_node($hg, visit_root<>);
  unlink(\HH\global_get('filename'));
}
