<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class AddressBook extends AddressBookContactCollection {

  private $ownerId;
  private $fbContacts = null;

  public function __construct($owner_id, array $contacts) {
    $this = $owner_id;
    parent::__construct($contacts);
  }

  public static function createFromLoader(AddressBookLoader $loader) {
    return new AddressBook($loader->getOwnerId(), $loader->getContacts());
  }

  public function getOwnerId() {
    return $this->ownerId;
  }

  public function addContact(AddressBookContact $contact) {
    invariant($this->ownerId > 0, 'AddressBook does not contain an owner.');

    if (!$contact->getOwnerId()) {
      $contact->setOwnerId($this->ownerId);
    }

    invariant(
	      $contact->getOwnerId() == $this->ownerId,
	      'AddressBookContact owner is not the same as AddressBook owner.');

    $remote_id = $contact->getRemoteId();
    invariant(!$remote_id, 'Contact already has a remote id. Cannot add');
    invariant(
	      !AddressBookLoader::isIncomingAssocType($contact->getAssocType()),
	      'Incoming assocs not supported in AddressBook.');

    return parent::addContact($contact);
  }
}

function f(){
}
