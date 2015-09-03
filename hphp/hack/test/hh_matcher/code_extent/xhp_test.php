<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class :fb:event:base-two-ents extends :fb:event:base-list-ents {

  final protected function render() {
    $ents = $this->:ents;
    $ents = array_values($ents);

    $display_count = count($ents);

    switch ($display_count) {
      case 0:
        return null;
      case 1:
        return $this->getProfileName(head($ents));
      case 2:
        return
          <fbt project="events" desc="List of names">
            <fbt:param name="first name">
              {$this->getProfileName($ents[0])}
            </fbt:param> and
            <fbt:param name="other name">
              {$this->getProfileName($ents[1])}
            </fbt:param>
          </fbt>;
      default:
        return
          <fbt project="events" desc="List of names">
            <fbt:param name="first name">
              {$this->getProfileName($ents[0])}
            </fbt:param> and
            <fbt:param name="number of others">
              {$this->renderOtherProfiles(array_slice($ents, 1))}
            </fbt:param>
          </fbt>;
    }
  }
}
