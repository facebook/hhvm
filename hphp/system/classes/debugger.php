<?php

interface DebuggerCommand {
  /**
   * Called when DebuggerClient needs to auto-complete. Inside this function,
   * one can call $client->addCompletion() with a list of strings or one of
   * those DebuggerClient::AUTO_COMPLETE_ constants.
   */
  public function onAutoComplete($client);

  /**
   * Called when DebuggerClient needs to displays help on the command. Inside
   * this function, one can call $client->help() and its different forms.
   *
   * @return  TRUE if helped, FALSE if any error
   */
  public function help($client);

  /**
   * How to process the command on client side.
   *
   * @return  TRUE for success, FALSE for failure
   */
  public function onClient($client);

  /**
   * How to process the command on server side.
   *
   * @return  TRUE for success, FALSE for failure
   */
  public function onServer($proxy);
}
