<?hh

// !!! Please contact devx_www oncall if this breaks. !!!
//
// "Fact parsing" is a core part of www infrastructure on devservers.


//
// Features to test
//

function f1() {}

class CL0 {
  // negative test
  function f3() {}
}

class CL1<T> {
}

interface I0 {
  // negative test
  function f4();
}

interface I1<T> {
}

trait TR0 {
  // negative test
  function f5() {}
}

trait TR1<T> {
}

const CO0 = 1;
const CO1 = "abc";
const CO2 = 1;
const CO3 = 'abc';
const CO4 = 2;

type TA0 = int;
newtype TA1 = int;
type TA2<T> = int;

abstract class CL2 extends CL0 implements I0, I1<int> {
  use TR0, TR1<int>;
}

interface I2 extends I0, I1<int> {
  require extends CL1;
}

trait TR2 implements I0, I1<int> {
  use TR0, TR1<int>;
  require extends CL1;
  require implements I2;
}

enum E0 : int {
  EV0 = 0;
  EV1 = 1;
}

//
// Execute single-threaded test
//
<<__EntryPoint>> function main(): void {
$source = file(__FILE__);
$source[0] = '<?hh namespace NS {';
$source[] = '}';
$namespace_file1 = tempnam(sys_get_temp_dir(), "factparsetest");
file_put_contents($namespace_file1, $source);

$source = file(__FILE__);
$source[0] = '<?hh namespace NS\NS {';
$source[] = '}';
$namespace_file2 = tempnam(sys_get_temp_dir(), "factparsetest");
file_put_contents($namespace_file2, $source);

try {
  $res = HH\facts_parse(
    null,
    varray[__FILE__, $namespace_file1, $namespace_file2],
    true, // allowHipHopSyntax
    false, // useThreads
  );
  var_dump($res);
  if ($res[__FILE__]['sha1sum'] === $res[$namespace_file1]['sha1sum']) {
    print "FAILED sha1sum\n";
  } else {
    print "PASSED sha1sum\n";
  }
} finally {
  unlink($namespace_file1);
  unlink($namespace_file2);
}


//
// Execute erroneous files test
//

$garbage_file = tempnam(sys_get_temp_dir(), "factparsetest");
file_put_contents($garbage_file, "<?hh }");

try {
  $res = HH\facts_parse(
    null,
    varray[$garbage_file, 'garbage_filename', sys_get_temp_dir()],
    true,
    false,
  );
  var_dump($res);
  if ($res[$garbage_file] === null
    && $res['garbage_filename'] === null
    && $res[sys_get_temp_dir()] === null
  ) {
    print "PASSED erroneous files test\n";
  } else {
    print "FAILED erroneous files test\n";
  }
} finally {
  unlink($garbage_file);
}

//
// Test path mangling
//

$simplefile = tempnam(sys_get_temp_dir(), "factparsetest");
file_put_contents($simplefile, "<?hh function f(){}");

try {
  $res = HH\facts_parse(
    sys_get_temp_dir(),
    varray[basename($simplefile), $simplefile, 'garbagename'],
    false,
    false,
  );
  if ($res[basename($simplefile)] === $res[$simplefile]
    && $res['garbagename'] === null
  ){
    print "PASSED path mangling test\n";
  } else {
    print "FAILED path mangling test\n";
  }
} finally {
  unlink($simplefile);
}


//
// Execute multi-threaded test
//

$many_files = varray[];
for ($i = 0; $i < 10000; $i++) {
  $many_files[] = tempnam(sys_get_temp_dir(), "factparsetest");
  file_put_contents($many_files[$i], $source);
}
$failed = false;
try {
  $facts = HH\facts_parse(null, $many_files, true, true);
  $first = serialize($facts[$many_files[0]]);
  foreach ($facts as $fact) {
    if ($first !== serialize($fact)) {
      print "FAILED multi-threaded test\n";
      $failed = true;
      break;
    }
  }
} finally {
  array_map('unlink', $many_files);
}
if (!$failed) {
  print "PASSED multi-threaded test\n";
}


//
// Test version get (not checking actual number)
//

if ((int)HH\ext_factparse_version()) {
  print "PASSED version get test\n";
} else {
  print "FAILED version get test\n";
}


//
// Check bad arg handling
//

try {
  HH\facts_parse(null, null, null, null);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  HH\facts_parse(null, varray[null], null, null);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  HH\facts_parse(null, varray[darray[]], null, null);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
