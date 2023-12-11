<?hh

const AR = vec[1,3,2];

//////////////////////////////////////////////////////////////////////
// Exception test cases with fault funclets and nested FPI regions of
// various complexity.

function func() :mixed{
}
function blar() :mixed{
  throw new Exception("Hi");
}

function foo() :mixed{

  foreach (AR as $y) {
    func(blar($y));
    echo "wat\n";
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case1() :mixed{ foo(); }

function foo2() :mixed{

  foreach (AR as $y) {
    func(12, new stdClass(), mt_rand(), blar($y) ? 1024 : -1);
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case2() :mixed{ foo2(); }

function foo3() :mixed{

  foreach (AR as $y) {
    func(12, new stdClass(), mt_rand(), func(blar($y)));
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case3() :mixed{ foo3(); }


function foo4() :mixed{

  foreach (AR as $y) {
    func(12, new stdClass(), mt_rand(), func(mt_rand(), blar($y)));
  }
  try {} catch (Exception $x) { echo "Bad\n"; }
}

function case4() :mixed{ foo3(); }
<<__EntryPoint>> function main(): void {
try { case1(); } catch (Exception $x) { echo "Good1\n"; }
try { case2(); } catch (Exception $x) { echo "Good2\n"; }
try { case3(); } catch (Exception $x) { echo "Good3\n"; }
try { case4(); } catch (Exception $x) { echo "Good4\n"; }
echo "Done\n";
}
