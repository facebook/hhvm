<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class BirthdayStoryPreparable extends Preparable {
  private $userIDs, $viewerContext, $users = MUST_PREPARE;

  public function __construct(ViewerContext $vc, $user_ids) {
    $this->viewerContext = $vc;
    $this->userIDs = IDAssert::allUid($user_ids);
  }

  public function getUsers() {
    return must_prepare($this->users);
  }

  public function getCurrentPosts() {
    $from = $this->viewerContext->getUserID();
    $to_users = array_keys($this->users);
    return DT('TickerBirthdayPost', $from)->getAll($to_users);
  }

  public function prepare($pass) {
    switch ($pass) {
      case 0:
        $this->waitFor(
          Ent::createEnforcingLoaderDynamic(
            'EntPersonalUser',
            $this->viewerContext,
            $this->userIDs,
            $this->users,
            varray[
              Ent::load('BasicInfo'),
              Ent::load('Birthday'),
              Ent::load('ProfilePic', PicSizeConst::SQUARE),
            ],
          ),
        );

        return true;
      case 1:
        $from = $this->viewerContext->getUserID();
        $to_users = array_keys($this->users);
        DT('TickerBirthdayPost', $from)->addAll($to_users);
    }
    return false;
  }
}

const MUST_PREPARE = null;

function must_prepare($_) {}
function DT($_, $_) {}

class Preparable {
  public function waitFor($_) {}
}
class ViewerContext {}
class Ent {
  public static function load(...$_) {}
  public static function createEnforcingLoaderDynamic(...$_) {}
}
enum PicSizeConst : string {
  SQUARE = 'SQUARE';
}
class IDAssert {
  public static function allUid($_) {}
}
