<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class AddressBook extends AddressBookContactCollection {

  private $ownerId;
  private $fbContacts = null;

  public function __construct($owner_id, array $contacts) {
    $this->ownerId = $owner_id;
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
      'AddressBookContact owner is not the same as AddressBook owner.',
    );

    $remote_id = $contact->getRemoteId();
    invariant(!$remote_id, 'Contact already has a remote id. Cannot add');
    invariant(
      !AddressBookLoader::isIncomingAssocType($contact->getAssocType()),
      'Incoming assocs not supported in AddressBook.',
    );

    return parent::addContact($contact);
  }
}

class AddressBookContactCollection {
  public function __construct($_) {}
  public function addContact($_) {}
}
class AddressBookLoader {
  public function getOwnerId() {}
  public function getContacts() {}
  public static function isIncomingAssocType($_) {}
}
class AddressBookContact {
  public function getOwnerId() {}
  public function setOwnerId($_) {}
  public function getRemoteId() {}
  public function getAssocType() {}
}
