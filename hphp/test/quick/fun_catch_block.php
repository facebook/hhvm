<?hh

class FunCatchBlock {
  public static function setMuteUnmuteState($jumbo_shrimp,
                                            $robot_car,
                                            $is_add) :mixed{
    try {
      $tagger = new ControllerObject(
                      $jumbo_shrimp,
                      Thing::SMS_MUTE,
                      $is_add);
      $tagger->setThreadIds(vec[$robot_car]);
      prep($tagger);
    } catch (GoodException $e) {
      // Some control flow within a catch block.  A case for the
      // emitter to get right.
      Logger('what')
        ->warn('This is a string %s %s: %s',
               $is_add ? 'mute' : 'unmute',
               $jumbo_shrimp->getAThing()->getUserID(),
               $e->getMessage());
      return false;
    }
    return true;
  }
}

class FunFunclet {
  protected async function genPayload() :Awaitable<mixed>{
    // This will generate a unreachable funclet-protected code.
    switch ($this->getAction()) {
      case 0:
        var_dump($this);
        return null;

      default:
        throw new SomeEx(ErrorCode::SOMETHING);
    }
    return null;
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
