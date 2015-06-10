<?hh

$s = strtolower('heLLo');

class C {
  public $x = 3;
  public $f = 'hi';
}

class B {
  public $f;
}

function apcOps($key, $val) {
  apc_store($key, $val);
  // need to fetch the value twice because under common
  // config settings object are serialized on first store
  // and APCObject (APCCollection) are created on second
  // fetch
  var_dump(apc_fetch($key));
  var_dump(apc_fetch($key));
}

echo "**** Object instance with simple properties\n";
$o = new C;
apcOps('key', $o);

echo "**** Vector containing object reference\n";
$c = Vector{1, 2, $o};
apcOps('key', $c);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj[] = 'hi';
$apcObj[0] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Vector containing non-refcounted values\n";
$c = Vector{1, 'hello'};
apcOps('key', $c);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj[] = 'hi';
$apcObj[0] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Vector containing simple values\n";
$c = Vector{true, $s};
apcOps('key', $c);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj[] = 'hi';
$apcObj[0] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Map containing non-refcounted values\n";
$m = Map{'a' => 'b', 'c' => 'd'};
apcOps('key', $m);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj['one'] = 'hi';
$apcObj['a'] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Map containing itself\n";
$m = Map{'a' => 'b', 'c' => $m};
apcOps('key', $m);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj['one'] = 'hi';
$apcObj['a'] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Map containing a vector\n";
$m = Map{'a' => 'b', 'c' => $c};
apcOps('key', $m);
echo "modify previous instance from APC\n";
$apcObj = apc_fetch('key');
$apcObj['one'] = 'hi';
$apcObj['a'] = 100;
$opcObj = null;
var_dump('MUTATED', $apcObj);
var_dump(apc_fetch('key'));

echo "**** Immutable Vector instance\n";
$c = ImmVector{1, 'hello'};
apcOps('key', $c);

echo "**** Immutable Map instance\n";
$m = ImmMap{'a' => 'b', 'c' => 'd'};
apcOps('key', $m);

echo "**** Object instance containg itself\n";
$o = new C;
$o->x = $o;
apcOps('key', $o);

echo "**** Object instance containg itself 2\n";
$o = new C;
$o->x = new B;
$o->x->f = $o;
apcOps('key', $o);

echo "**** Object instance containg itself 3\n";
$o = new C;
$o->x = new B;
$o->x->f = Vector{};
$o->x->f[] = $o;
apcOps('key', $o);

echo "**** Object instance containg itself 4\n";
$o = new C;
$o->x = new B;
$o->x->f = Vector{};
$o->x->f[] = $o->x;
apcOps('key', $o);

echo "**** Object instance containg itself 5\n";
$o = new C;
$o->x = new B;
$o->x->f = Vector{};
$o->x->f[] = new B;
apcOps('key', $o);

echo "**** Object instance containg Vector instace with simple values\n";
$o = new C;
$o->x = new B;
$o->x->f = Vector{};
$o->x->f[] = 10;
$o->x->f[] = 'hi';
$o->x->f[] = strtolower('heLLo');
apcOps('key', $o);

echo "**** Object instance containg collections instaces\n";
$o = new C;
$o->x = new B;
$o->x->f = Map{};
$o->x->f['k'] = 10;
$o->x->f['k1'] = 'hi';
$o->x->f['k2'] = Vector{};
$o->x->f['k2'][] = 100;
$o->x->f['k2'][] = 'hello';
apcOps('key', $o);

echo "**** Set instance\n";
$s = Set{ 1, 'hi', 4, $s};
apcOps('key', $s);

echo "**** Set instance with int-like strings\n";
$s = Set{ 123, '123'};
apcOps('key', $s);

echo "**** Make sure delete is ok\n";
apc_delete('key');
