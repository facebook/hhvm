<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class SimpleSoftCache2<Tk1, Tk2, Tv> {

  private dict<Tk1, dict<Tk2, Tv>> $cache = dict[];

  public function __construct()[] {}

  public function get(Tk1 $key): ?dict<Tk2, Tv> {
    return idx($this->cache, $key);
  }

  public function set(Tk1 $key, dict<Tk2, Tv> $value): void {
    $this->cache[$key] = $value;
  }

  public function remove(Tk1 $key): void {
    unset($this->cache[$key]);
  }

  public function nuke(): void {
    $this->cache = dict[];
  }

  public function get2(Tk1 $key1, Tk2 $key2): ?Tv {
    $result = isset($this->cache[$key1][$key2])
      ? $this->cache[$key1][$key2]
      : null;
    return $result;
  }

  public function set2(Tk1 $key1, Tk2 $key2, Tv $value): void {
    if (!array_key_exists($key1, $this->cache)) {
      $this->cache[$key1] = dict[];
    }
    $this->cache[$key1][$key2] = $value;
  }

  public function remove2(Tk1 $key1, Tk2 $key2): void {
    unset($this->cache[$key1][$key2]);
  }
}


<<__EntryPoint>>
function main_softcache() :mixed{
$c = new SimpleSoftCache2;
$c->set('a', dict[1=>2,'1'=>'2']);
$c->set2('a', 'b', 'c');
$c->set2('d', 'e', 'f');
$c->set2('x', 'y', 'z');
$c->remove('d');
$c->remove2('a', 1);
var_dump($c->get('a'));
var_dump($c->get2('x', 'y'));
$c->nuke();
var_dump($c->get('a'));
}
