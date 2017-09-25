<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Character {
  private $actor;

  function setActor(?string $actor) {
    $this->actor = $actor;
  }
}

$sherlock = new Character;
$sherlock->setActor('Bumblebee Cumberbund');
$sherlock->setActor(5);
echo "Done\n";
