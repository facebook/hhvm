<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// Reference implementations:
//  www/flib/core/contextprop/ContextManager.php
//  www/flib/core/contextprop/init/WWWContextPropEventHandler.php
//  www/flib/thrift/core/protocol/__tests__/TCompactProtocolTest.php
//  www/flib/batch_queue/queue/impl/__tests__/ScribeBatchQueueTest.php

final class ThriftContextPropState {
  private static ?ThriftContextPropState $instance = null;
  private ThriftFrameworkMetadata $storage;
  private dict<int, string> $serializedStorage;

  private function __construct()[write_props, zoned_shallow] {
    $this->storage = ThriftFrameworkMetadata::withDefaultValues();
    $this->serializedStorage = dict[];
  }

  /**
   * Initializes ThriftContextPropState from Headers
   */
  public static function initFromHeaders()[defaults]: void {
    $tfm = HTTPHeaders::get()->getHeader(HTTPRequestHeader::THRIFT_FMHK);
    self::initFromString($tfm);
  }

  // If anything changes with the ThriftFrameworkMetadata, throw out the
  // serialized representation.
  private function dirty()[write_props]: void {
    if (!C\is_empty($this->serializedStorage)) {
      $this->serializedStorage = dict[];
    }
  }

  public function getSerialized()[write_props, zoned_shallow]: string {
    // We cache the serialized representation per origin ID, so that we only
    // ever serialize it once unless something other than origin ID has changed.
    $origin_id = $this->getOriginId();
    if ($origin_id === null) {
      // This is higher than the max allowable origin ID, but it's a valid key
      // for the dict.
      $origin_id = PHP_INT_MAX;
    }
    if (!C\contains_key($this->serializedStorage, $origin_id)) {
      $this->serializedStorage[$origin_id] = self::serialize($this->storage);
    }
    return $this->serializedStorage[$origin_id];
  }

  public function getSerializedWithOriginIDOverride(
    ?int $origin_id,
  )[zoned_local]: string {
    $original_origin_id = $this->getOriginId();
    // To get TFM for a given origin ID, we store the existing global origin ID,
    // change the TFM's origin ID, serialize it, then change it back to the
    // original value. This is all a sync operation, so it's safe.
    $this->setOriginId($origin_id);
    $serialized = $this->getSerialized();
    $this->setOriginId($original_origin_id);
    return $serialized;
  }

  private static function serialize(
    ThriftFrameworkMetadata $tfm,
  )[write_props, zoned_shallow]: string {
    $buf = new TMemoryBuffer();
    $prot = new TCompactProtocolAccelerated($buf);
    $tfm->write($prot);
    $s = $buf->getBuffer();
    return Base64::encode($s);
  }

  /**
   * Initizes ThriftContextPropState from a string (base64-encoded
   * compact-serialized ThriftFrameworkMetadata).
   */
  public static function initFromString(?string $s)[defaults]: void {
    try {
      $rid_set = false;
      if ($s !== null) {
        $transport = Base64::decode($s);
        $buf = new TMemoryBuffer($transport);
        $prot = new TCompactProtocolAccelerated($buf);
        $tfm = ThriftFrameworkMetadata::withDefaultValues();
        $tfm->read($prot);

        $rid = $tfm->request_id;
        if ($rid !== null && !Str\is_empty($rid)) {
          $rid_set = true;
        }

        self::get()->storage = $tfm;
        self::get()->dirty();
      }
    } catch (\Exception $ex) {
      // Swallow the error, rather than nuke the whole request
      FBLogger('thrift')
        ->event(__CLASS__.'_'.__METHOD__.'_exception')
        ->catching($ex)
        ->mustfix('swallowing deserialization error: %s', $ex->getMessage());
    }
    if (!$rid_set) {
      self::get()->storage->request_id = self::generateRequestId();
      self::get()->dirty();
    }
  }

  /**
   * Returns the ThriftContextPropState singleton
   */
  public static function get(
  )[globals, write_props, zoned_shallow]: ThriftContextPropState {
    if (self::$instance === null) {
      self::$instance = new ThriftContextPropState();
    }
    return self::$instance;
  }

  public static function getReadonlyIfInitialized(
  )[leak_safe]: readonly ?ThriftContextPropState {
    return readonly self::$instance;
  }

  /**
   * Returns true if any fields were set with non-empty values.
   */
  public function isSet()[write_props, zoned_shallow]: bool {
    foreach (ThriftFrameworkMetadata::FIELDMAP as $field_name => $_) {
      /* this assume that all fields are optional */
      /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
      if (!$this->storage->$field_name is null) {
        return true;
      }
    }
    return false;
  }

  /**
   * Accessors
   */
  public readonly function getRequestId()[leak_safe]: string {
    if ($this->storage->request_id is null) {
      return "";
    }
    return ($this->storage->request_id as string);
  }

  public function setRequestId(?string $s)[write_props]: void {
    $this->storage->request_id = $s;
    $this->dirty();
  }

  public readonly function getOriginId()[leak_safe]: ?int {
    if ($this->storage->origin_id is null) {
      return null;
    }
    return $this->storage->origin_id as int;
  }

  public function setOriginId(?int $id)[write_props]: void {
    // We do not dirty cache in this case, because the cache key is origin ID.
    $this->storage->origin_id = $id;
  }

  public readonly function getRegionalizationEntity()[leak_safe]: ?int {
    if ($this->storage->baggage is null) {
      return null;
    }
    $re = $this->storage->baggage?->regionalization_entity;
    if ($re is null) {
      return null;
    }
    return ($re as int);
  }

  public function setRegionalizationEntity(?int $re)[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();

    $baggage = $this->storage->baggage as nonnull;
    $baggage->regionalization_entity = $re;
    $this->dirty();
  }

  public function getTraceContextFlags1()[]: ?int {
    if ($this->getBaggage() is nonnull && $this->getTraceContext() is nonnull) {
      return $this->storage->baggage?->trace_context?->flags1;
    }
    return null;
  }

  public function setTraceContextFlags1(int $flags1)[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();
    $baggage = $this->storage->baggage as nonnull;
    $baggage->trace_context =
      $baggage->trace_context ?? ContextProp\TraceContext::withDefaultValues();
    $baggage->trace_context->flags1 = $flags1;
    $this->dirty();
  }

  public function getBaggageFlags1()[]: ?int {
    if ($this->getBaggage() is nonnull) {
      return $this->storage->baggage?->flags1;
    }
    return null;
  }

  public function setBaggageFlags1(int $flags1)[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();

    $baggage = $this->storage->baggage as nonnull;
    $baggage->flags1 = $flags1;
    $this->dirty();
  }

  public function getBaggage()[]: ?ContextProp\Baggage {
    return $this->storage->baggage;
  }

  public function setBaggage(?ContextProp\Baggage $baggage)[write_props]: void {
    $this->storage->baggage = $baggage;
    $this->dirty();
  }

  public function getModelInfo()[]: ?ContextProp\ModelInfo {
    return $this->getBaggage()?->model_info;
  }

  public function getModelTypeId()[]: ?int {
    return $this->getModelInfo()?->get_model_type_id();
  }

  // user id getters
  public function getUserIds()[]: ?ContextProp\UserIds {
    return $this->getBaggage()?->user_ids;
  }

  public function getFBUserId()[]: ?int {
    return $this->getUserIds()?->fb_user_id;
  }

  public function getIGUserId()[]: ?int {
    return $this->getUserIds()?->ig_user_id;
  }

  // user id setters
  public function setUserIds(
    ?ContextProp\UserIds $user_ids,
  )[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();

    $baggage = $this->storage->baggage as nonnull;
    $baggage->user_ids = $user_ids;
    $this->dirty();
  }

  public function setFBUserId(int $fb_user_id)[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();
    $baggage = $this->storage->baggage as nonnull;

    $baggage->user_ids =
      $baggage->user_ids ?? ContextProp\UserIds::withDefaultValues();
    $baggage->user_ids->fb_user_id = $fb_user_id;
    $this->dirty();
  }

  public function setIGUserId(int $ig_user_id)[write_props]: void {
    $this->storage->baggage =
      $this->storage->baggage ?? ContextProp\Baggage::withDefaultValues();
    $baggage = $this->storage->baggage as nonnull;

    $baggage->user_ids =
      $baggage->user_ids ?? ContextProp\UserIds::withDefaultValues();
    $baggage->user_ids->ig_user_id = $ig_user_id;
    $this->dirty();
  }

  public function getTraceContext()[]: ?ContextProp\TraceContext {
    if ($this->storage->baggage is nonnull) {
      $trace_context = $this->storage->baggage?->trace_context;
      return $trace_context;
    }
    return null;
  }

  public static function getOriginIdResolver()[]: (function()[leak_safe]: int) {
    return ()[leak_safe] ==> (
      readonly ThriftContextPropState::getReadonlyIfInitialized()
    )?->getOriginId() ??
      MCPProductID::UNKNOWN;
  }

  public function getExperimentIds()[]: vec<int> {
    $ret = $this->storage->experiment_ids;
    if ($ret !== null) {
      return $ret;
    }
    return vec[];
  }

  public function addExperimentId(int $eid)[write_props]: void {
    $v = $this->getExperimentIds();
    $v[] = $eid;
    $this->storage->experiment_ids = $v;
    $this->dirty();
  }

  public function setExperimentIds(vec<int> $eids)[write_props]: void {
    $this->storage->experiment_ids = $eids;
    $this->dirty();
  }

  public function getPrivacyUniverse()[]: ?int {
    return $this->storage->privacyUniverse;
  }

  public function setPrivacyUniverse(?int $universe)[write_props]: void {
    $this->storage->privacyUniverse = $universe;
    $this->dirty();
  }

  public function getRequestPriority()[]: ?RequestPriority {
    return $this->storage->request_priority;
  }

  public function setRequestPriority(
    ?RequestPriority $request_priority,
  )[write_props]: void {
    $this->storage->request_priority = $request_priority;
    $this->dirty();
  }

  public function clear(): void {
    $this->serializedStorage = dict[];
    $this->storage = ThriftFrameworkMetadata::withDefaultValues();
  }

  public static function generateRequestId(): string {
    return SecureRandom\string(16);
  }

  public function isBaggageFlags1Set(
    ContextProp\BaggageFlags1 $flag_name,
  ): bool {
    if ($flag_name == ContextProp\BaggageFlags1::NOT_ALLOWED) {
      FBLogger(__CLASS__, __METHOD__)->mustfix(
        'Invalid flag name %s',
        ContextProp\BaggageFlags1::getNames()[$flag_name],
      );
      return false;
    }

    $flags1 = ThriftContextPropState::get()->getBaggageFlags1();
    if ($flags1 is null) {
      return false;
    }
    return (($flags1 >> (int)$flag_name) & 1) === 1;
  }

  public function setBaggageFlags1ByName(
    ContextProp\BaggageFlags1 $flag_name,
  ): void {
    if ($flag_name == ContextProp\BaggageFlags1::NOT_ALLOWED) {
      FBLogger(__CLASS__, __METHOD__)->mustfix(
        'Invalid flag name %s',
        ContextProp\BaggageFlags1::getNames()[$flag_name],
      );
    }

    $flags1 = ThriftContextPropState::get()->getBaggageFlags1() ?? 0;
    $flags1 |= (1 << (int)$flag_name); //set bit at flag position to 1
    $this->setBaggageFlags1($flags1);
  }

  public function clearBaggageFlags1ByName(
    ContextProp\BaggageFlags1 $flag_name,
  ): void {
    if ($flag_name == ContextProp\BaggageFlags1::NOT_ALLOWED) {
      FBLogger(__CLASS__, __METHOD__)->mustfix(
        'Invalid flag name %s',
        ContextProp\BaggageFlags1::getNames()[$flag_name],
      );
    }

    $flags1 = ThriftContextPropState::get()->getBaggageFlags1();
    if ($flags1 is null) {
      return;
    }
    $flags1 &= ~(1 << (int)$flag_name); //set bit at flag position to 0
    $this->setBaggageFlags1($flags1);
  }

  public function getTFMCopy(): ThriftFrameworkMetadata {
    $tfm_copy = ThriftFrameworkMetadata::withDefaultValues();
    $tfm_copy->request_id = $this->storage->request_id;
    $tfm_copy->origin_id = $this->storage->origin_id;
    $tfm_copy->deadline_ms = $this->storage->deadline_ms;
    $tfm_copy->experiment_ids = $this->storage->experiment_ids;
    $tfm_copy->routingKey = $this->storage->routingKey;
    $tfm_copy->shardId = $this->storage->shardId;
    $tfm_copy->loggingContext = $this->storage->loggingContext;
    $tfm_copy->processingTimeout = $this->storage->processingTimeout;
    $tfm_copy->overallTimeout = $this->storage->overallTimeout;
    $tfm_copy->privacyUniverse = $this->storage->privacyUniverse;
    $tfm_copy->request_priority = $this->storage->request_priority;
    $tfm_copy->client_id = $this->storage->client_id;
    $tfm_copy->baggage = $this->storage->baggage;

    return $tfm_copy;
  }

}
