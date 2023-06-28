<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Blob {
  function getThing() :mixed{}
}

class thing {
  private $blob;

  async function getBlob() :Awaitable<mixed>{
    if (!$this->blob) $this->blob = new Blob;
    return $this->blob;
  }

  async function wat() :Awaitable<mixed>{
    $blob = await $this->getBlob();
    $blob->getThing();
  }
}


<<__EntryPoint>>
function main_await_predict() :mixed{
$t = new thing;
for ($i = 0; $i < 500; ++$i) {
  $t->wat();
}
echo "Done\n";
}
