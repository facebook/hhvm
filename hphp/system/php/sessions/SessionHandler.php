<?php

/**
 * http://php.net/class.SessionHandler
 *
 * SessionHandler a special class that can be used to expose the current
 * internal PHP session save handler by inheritance. There are six methods
 * which wrap the six internal session save handler callbacks (open, close,
 * read, write, destroy and gc). By default, this class will wrap whatever
 * internal save handler is set as as defined by the session.save_handler
 * configuration directive which is usually files by default.
 */
class SessionHandler implements SessionHandlerInterface {
  public function __construct() { }

  <<__Native>>
  private function hhopen(string $save_path, string $session_id): bool;

  public function open($save_path, $session_id) {
    return $this->hhopen($save_path, $session_id);
  }

  <<__Native>>
  private function hhclose(): bool;

  public function close() {
    return $this->hhclose();
  }

  <<__Native>>
  private function hhread(string $session_id): string;

  public function read($session_id) {
    return $this->hhread($session_id);
  }

  <<__Native>>
  private function hhwrite(string $session_id, string $data): bool;

  public function write($session_id, $data) {
    return $this->hhwrite($session_id, $data);
  }

  <<__Native>>
  private function hhdestroy(string $session_id): bool;

  public function destroy($session_id) {
    return $this->hhdestroy($session_id);
  }

  <<__Native>>
  private function hhgc(int $maxlifetime): bool;

  public function gc($maxlifetime) {
    return $this->hhgc($maxlifetime);
  }
}
