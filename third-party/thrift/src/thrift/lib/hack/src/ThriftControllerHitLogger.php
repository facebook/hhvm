<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
