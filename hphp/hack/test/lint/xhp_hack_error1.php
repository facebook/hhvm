<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class :derpyclass {

  protected function renderMeDerpy() {
    $select = <select name={$this->:name} />;
    $select->appendChild(
      <option value={$default}
              selected={XhpHackError::attributeExpectsBool("selected")}>
      Custom ({$default})
      </option>);
      return null;
  }

}
