<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class PlaceMutation implements IPlaceMutation {

  private $preMutationPlace, $visibilityLock, $location,
    $sentryCategory, $appID, $uri, $visibilityInfo,
    $relatedUIDs;

  public function setRadiusMeters($x) { }
  public function getMutationOperation() { }
  public function changesVisibility() { }

  public function setPreMutationPlace(EntPlace $place) {
    $this->preMutationPlace = $place;

    $this->visibilityLock = $place->getVisibilityLockObject();
    if ($place->getRadiusMeters()) {
      $this->setRadiusMeters($place->getRadiusMeters());
    }
    return $this
      ->setLocation($place->getLocation())
      ->setPlaceID($place->getID())
      ->setUnindexedPlace($place->getUnindexedPlace())
      ->setVisibilityInfo($place->getVisibilityInfo())
      ->setRankBoost($place->getRankBoost());
  }

  public function hasPreMutationPlace() {
    return $this->preMutationPlace !== null;
  }

  public function getPreMutationPlace() {
    if (!$this->preMutationPlace) {
      throw new Exception('Original place must be provided');
    }
    return $this->preMutationPlace;
  }

  public function getLocation() {
    return $this->location;
  }

  public function setLocation($location) {
    // TODO (beng): Task #219561: Add a call to PlacesAssert::isGeoJSON() here
    // once that function is written.
    $this->location = $location;
    return $this;
  }




  public function setDescription($description) {
    return $this;
  }


  public function setCreationDBID(int $creation_dbid) {
    return $this;
  }

  public function getSentryCategory() {
    return $this->sentryCategory;
  }

  public function setSentryCategory($sentry_category) {
    $this->sentryCategory = ArgAssert::isNonEmptyString($sentry_category);
    return $this;
  }

  public function getAppID() {
    return $this->appID;
  }

  public function setAppID($app_id) {
    IDAssert::isOidOrNull($app_id);
    $this->appID = $app_id;
    return $this;
  }

  public function setURI($uri) {
    $this->uri = ArgAssert::isNonEmptyString($uri);
    return $this;
  }

  public function getVisibilityInfo() {
    return $this->visibilityInfo;
  }

  public function setVisibilityInfo($visibility_info) {
    $visibility_info = $visibility_info ? $visibility_info : array();
    $this->visibilityInfo = ArgAssert::isArray($visibility_info);
    PlaceVisibilityPreparable::validateVisibilityInfoParams($visibility_info);

    switch ($this->getMutationOperation()) {
      case IPlaceMutation::OPERATION_CREATE:
        break;
      default:
        if (!$this->hasPreMutationPlace()) {
          throw new Exception(
            'Cannot Set Visibility without setting old Place data first'
          );
        }
        break;
    }
    if ($this->changesVisibility()) {
      switch ($this->getMutationOperation()) {
        case IPlaceMutation::OPERATION_CREATE:
        case IPlaceMutation::OPERATION_MUTATE_VISIBILITY:
          // TODO (dhui,redstone): Remove this when RESTORE operations
          // no longer mutate visibility after task #391587
        case IPlaceMutation::OPERATION_RESTORE:
          break;
        default:
          throw new Exception(
            'Action '.$this->getMutationOperation().
            ' does not support visibility changes'
          );
      }
    }

    return $this;
  }

  public function getVisibilityLockObject() {
    return $this->visibilityLock;
  }

  public function getRelatedUIDs() {
    return $this->relatedUIDs;
  }

  /**
   * Sets state to be sent to the Proximity Server about which users
   * (and their friends) should be able to see this Place
   */
  public function setRelatedUIDs($related_uids) {
    if ($related_uids === null) {
      return $this;
    }
    ArgAssert::allNumeric($related_uids);
    // Don't set logged out viewer IDs (in the case of anonymous Place creation)
    $this->relatedUIDs = array_filter($related_uids);
    return $this;
  }
}
