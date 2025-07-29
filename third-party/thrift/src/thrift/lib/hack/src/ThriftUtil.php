<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

<<Oncalls('thrift')>> // @oss-disable
abstract final class ThriftUtil {

  const keyset<classname<IThriftShapishStruct>> INTISH_CAST_STRUCTS = keyset[
    nameof AdReviewDocNNModelConfig,
    nameof AdReviewDocNNModels,
    nameof AdReviewModelTransformationConfig,
    nameof BEAM_BEAMDynamicModelThresholdStructs,
    nameof BEAM_BEAMPerGroupModelThresholdConfig,
    nameof CallerNameToMeta,
    nameof Dataeng_MappingConfig,
    nameof EntityProviders,
    nameof FeatureBank_BreakdownValuemap,
    nameof FeatureBank_Parameters,
    nameof GenericIrisGatekeeperTopicConfig,
    nameof Laser_CommerceTagRelatedQueries_CommerceTagRelatedQueries,
    nameof MarketplaceAdsMTSecondaryMapping,
    nameof ModelActionableComponentConfig,
    nameof ModelToPerCompTypeThresholdsWrapper,
    nameof OnDemandConfig_MatchmakingSpec,
    nameof OpsStreamEvent,
    nameof QRTModelMapping,
    nameof SignalMappingCollectionV2,
    nameof WebsiteEventContextParamStruct,
    nameof galileo_GalileoClientViewShardedMap,
    nameof galileo_NoTransform,
    nameof ig_launcher_Whitelist,
    nameof mocha_TraitEntry,
    nameof pytext_preprocessing_PreprocessingMap,
    nameof story_interaction_FLMFBStoriesFeatureBreakdownMappingConfig,
    nameof uipbreakdown_WeightedFormulaValue,
    nameof uma_AlertBaseBreakdownConfig,
    nameof uma_DynamicBreakdownSamplingConfig,
    nameof Laser_OwnershipAggregatorFinalPredictions_OwnershipDiscoveryAggregatorFinalPredictionsSignal,
    nameof RolloutConfig,
    nameof qe_NewUniverse,
    nameof SignalsIntegrityParameters,
    nameof qe1_UniverseOverrides,
    nameof Bloks_Versioning_VersionedProp,
    nameof IDGraphPixelMatchResult,
    nameof uma_ChangedExperimentConfig,
    nameof uma_DynamicBreakdownConfig,
  ];

  /**
   * This is a migration strategy used to preserve pre-existing behavior in
   * Hack/PHP that used to cause int-like strings to be converted to ints
   * when used as array keys. More info: https://fburl.com/intishcast
   */
  public static function toDArray<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $traversable,
    classname<IThriftShapishStruct> $caller,
  )[]: dict<Tk, Tv> {
    $caller ??= '<unknown>';
    if (
      !C\contains_key(
        self::INTISH_CAST_STRUCTS,
        HH_FIXME::tryClassToClassname($caller),
      )
    ) {
      return dict($traversable);
    }

    $result = dict[];
    foreach ($traversable as $key => $value) {
      $result[PHPArrayism::preserveLegacyKey($key)] = $value;
    }
    return HH\FIXME\UNSAFE_CAST<dict<arraykey, Tv>, dict<Tk, Tv>>(
      $result,
      'FIXME[4110] maintain lie',
    );
  }

  public static function getUnionTypeFromShape<
    TUnion as IThriftUnion<TEnum> as IThriftShapishStruct with {
      type TShape = TShape },
    reify TEnum,
    TShape as shape(...),
  >(TShape $shape)[]: TEnum {
    $found_type = null;
    foreach (Shapes::toDict($shape) as $key => $val) {
      if ($val is nonnull) {
        invariant($found_type is null, 'Union type can only contain one value');
        $found_type = $key;
      }
    }
    $union_enum_name =
      ArgAssert::isEnumname(HH\ReifiedGenerics\get_classname<TEnum>());
    $union_enum_pointer = HH\classname_to_class($union_enum_name);
    return $union_enum_pointer::getValues()[
      $found_type is null ? '_EMPTY_' : (string)$found_type
    ];
  }

  public static function requireSameType<T1, T2>()[]: void where T1 = T2 {}

  // Replaces dynamic field access in use cases to a centralized helper method.
  public static async function genToValueMap(
    IThriftStruct $thrift_object,
  )[zoned_shallow]: Awaitable<dict<string, mixed>> {
    $is_union = $thrift_object is IThriftUnion<_>;
    $obj_spec = $thrift_object::SPEC;
    $thrift_object as dynamic;
    $obj_spec = Dict\from_values($obj_spec, $field_spec ==> $field_spec['var']);
    return await Dict\map_async(
      $obj_spec,
      async $field_spec ==> {
        $is_wrapped = Shapes::idx($field_spec, 'is_wrapped', false);
        $field_name = $field_spec['var'];
        if ($is_wrapped || $is_union) {
          $acc_meth = 'get_'.$field_name;
          // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
          // Wrapped fields and unions have a getter.
          $val = $thrift_object->$acc_meth();
          if ($is_wrapped) {
            // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
            // Wrappers have genUnwrap method.
            $val = await $val?->genUnwrap();
          }
        } else {
          $val = $thrift_object->$field_name;
        }
        return $val;
      },
    );
  }

  public static async function genField(
    IThriftStruct $thrift_object,
    ThriftStructTypes::TFieldSpec $field_spec,
  )[zoned_shallow]: Awaitable<dynamic> {
    $is_union = $thrift_object is IThriftUnion<_>;
    $is_wrapped = Shapes::idx($field_spec, 'is_wrapped', false);
    $field_name = $field_spec['var'];
    $thrift_object as dynamic;
    if ($is_wrapped || $is_union) {
      $acc_meth = 'get_'.$field_name;
      // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
      // Wrapped fields and unions have a getter.
      $val = $thrift_object->$acc_meth();
      if ($is_wrapped) {
        // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
        // Wrappers have genUnwrap method.
        $val = await $val?->genUnwrap();
      }
    } else {
      $val = $thrift_object->$field_name;
    }
    return $val;
  }

  public static function getFieldValue(
    IThriftSyncStruct $thrift_object,
    string $field_name,
  )[]: dynamic {
    $is_union = $thrift_object is IThriftUnion<_>;
    $thrift_object as dynamic;
    if ($is_union) {
      $acc_meth = 'get_'.$field_name;
      // @lint-ignore DYNAMICALLY_INVOKING_TARGETS_CONSIDERED_HARMFUL
      // Wrapped fields and unions have a getter.
      $val = $thrift_object->$acc_meth();
    } else {
      $val = $thrift_object->$field_name;
    }
    return $val;
  }

  public static async function genSetField(
    IThriftStruct $thrift_object,
    ThriftStructTypes::TFieldSpec $field_spec,
    dynamic $value,
  ): Awaitable<void> {
    $thrift_object
      as dynamic; // -> setter method and field accesses are dynamic
    $field_name = $field_spec['var'];
    $is_wrapped = Shapes::idx($field_spec, 'is_wrapped', false);
    if ($thrift_object is IThriftUnion<_>) {
      if ($value !== null) {
        $acc_meth = 'set_'.$field_name;
        $thrift_object->$acc_meth($value);
      }
    } else if ($is_wrapped) {
      $acc_meth = 'get_'.$field_name;
      $wrapper = $thrift_object->$acc_meth();
      await $wrapper->genWrap($value);
    } else {
      $thrift_object->$field_name = $value;
    }
  }
}

final class ThriftAdapterVerifier<TStructThriftType, TStructHackType> {
  public function requireSameReturnType<TAdapterTThriftType, TAdapterTHackType>(
    (function(TAdapterTThriftType, int, IThriftStruct): TAdapterTHackType) $_,
    (function(TAdapterTHackType): TAdapterTThriftType) $_,
  )[]: void
  where
    TStructThriftType = TAdapterTThriftType,
    TStructHackType = TAdapterTHackType {}
}
