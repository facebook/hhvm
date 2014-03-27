<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class BirthdayStoryPreparable extends Preparable {
  private
    $userIDs,
    $viewerContext,
    $users = MUST_PREPARE;

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
        $this->waitFor(Ent::createEnforcingLoaderDynamic(
          'EntPersonalUser',
          $this->viewerContext,
          $this->userIDs,
          $this->users,
          array(
            Ent::load('BasicInfo'),
            Ent::load('Birthday'),
            Ent::load('ProfilePic', PicSizeConst::SQUARE),
          )
        ));

        return true;
      case 1:
        $from = $this->viewerContext->getUserID();
        $to_users = array_keys($this->users);
        DT('TickerBirthdayPost', $from)->addAll($to_users);
    }
    return false;
  }
}



