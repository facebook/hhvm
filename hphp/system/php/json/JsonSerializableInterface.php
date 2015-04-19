<?php

/**
 * Objects implementing JsonSerializable can customize their JSON
 * representation when encoded with json_encode().
 */
interface JsonSerializable {
  /**
   * Specify data which should be serialized to JSON
   *
   * @return mixed - Returns data which can be serialized by
   *   json_encode(), which is a value of any type other than a resource.
   */
  public function jsonSerialize();

}
