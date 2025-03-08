<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

<<Oncalls('alite')>>
final class ThriftControllerHitLogger {

  public static function logControllerHit(
    IXThriftController $controller,
    AliteControllerHitLoggerStage $stage,
  ): void {
    // We only want to run the Controller logger if it's the PRE_CHECKS (aka first)
    // stage AND the controller actually has a log implementation
    // Otherwise save ourselves the PSP boot-up CPU
    if (
      $controller is IXThriftControllerWithCustomLogging &&
      $stage === AliteControllerHitLoggerStage::PRE_CHECKS
    ) {
      PSP()->startLaterAndWaitFor(async () ==> {
        $exception = PHPFatal::getFatalException();
        await $controller->genLogRequest($exception);
      });
    }

    self::logControllerClassHit(Classnames::getx($controller), $stage);
  }

  public static function logControllerClassHit(
    classname<IXThriftController> $class,
    AliteControllerHitLoggerStage $stage,
  ): void {
    // The Entity and Key values are holdovers from when
    // Thrift was run through Alite. Since these are still
    // the active source of truth for usage data, we'll leave
    // them as-is
    $is_jest_e2e = WebDriverConfig::isJestE2ETestRun();
    $entity = !$is_jest_e2e ? 'alite' : 'alite.jest_e2e';
    $key = 'controller.hit.'.$stage.$class;

    // Do not sample this; it's used for usage (and dead code) detection
    CategorizedOBC::typedGet(ODSCategoryID::ODS_ALITE)
      ->bumpEntityKey($entity, $key);
  }

}
