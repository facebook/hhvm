<?hh

abstract class WebAsyncPluginController {
  /* HH_FIXME[4030] */
  protected function getFallbackURL() {
    throw new Exception();
  }
}
abstract class WebAsyncPostPluginController extends WebAsyncPluginController {
  protected function getFallbackURL(): ?string {
    $unsigned_fallback = parent::getFallbackURL();
    if ($unsigned_fallback) {
      return "";
    }
    return null;
  }
}
