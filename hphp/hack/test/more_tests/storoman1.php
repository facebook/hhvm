<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class WebHelp2Controller {
  public static function empty(): ?WebHelp2Controller {
    return null;
  }
}
class WebHelp2HubController extends WebHelp2Controller {}
class WebHelp2PageController extends WebHelp2Controller {}
class WebHelp2FrequentQuestionController extends WebHelp2Controller {}



function test(string $huri, bool $cmsid): void {
  $controller = WebHelp2Controller::empty();

  switch ($huri) {
  case 'hub':
    $controller = new WebHelp2HubController();
    break;
  case 'page':
    $controller = new WebHelp2PageController();
    break;
  case 'faq':
    $controller = new WebHelp2FrequentQuestionController();
    break;
  default:
    throw new Exception('You shouldn\'t be here');
  }
}
