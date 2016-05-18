<?hh // strict

interface IDataFlowElasticSearchType<TID1 as string> {
  static function getAllFields(
  ): ConstVector<classname<IDataFlowElasticSearchFieldAbstractValue<TID1>>>;

}
interface IDataFlowElasticSearchFieldAbstractValue<TID2>
  extends
    IDataFlowElasticSearchField<TID2, this::TValue>,
    IDataFlowElasticSearchBaseFieldAbstractValue<TID2> {
  static function getAnalyzers(): Map<string, array<string, mixed>>;
  static function getFilters(): Map<string, array<string, mixed>>;
}

interface IDataFlowElasticSearchField<TID3, +TValue>
  extends IDataFlowElasticSearchBaseField<TID3, TValue> {}

interface IDataFlowElasticSearchBaseFieldAbstractValue<TID4>
  extends IDataFlowElasticSearchBaseField<TID4, this::TValue> {}

interface IDataFlowElasticSearchBaseField<TID5, +TValue>
  extends IDataFlowElasticSearchTypeMappingField<TID5> {}

interface IDataFlowElasticSearchTypeMappingField<TID6> {}
abstract class DataFlowElasticSearchIndex {

  private static function getAnalyzersAndFilters<TID7 as string>(
    classname<IDataFlowElasticSearchType<TID7>> $type,
  ): (Map<string, array<string, mixed>>, Map<string, array<string, mixed>>) {
    return
      self::reduceAnalyzersAndFilters(
        $type::getAllFields()
          ->map(
            (
              classname<IDataFlowElasticSearchFieldAbstractValue<TID7>> $field,
            ) ==> tuple($field::getAnalyzers(), $field::getFilters()),
          ),
      );
  }

  private static function reduceAnalyzersAndFilters(
    ConstVector<(Map<string, array<string, mixed>>,
    Map<string, array<string, mixed>>,
    )> $analyzers_and_filters,
  ): (Map<string, array<string, mixed>>, Map<string, array<string, mixed>>) {
    return tuple(Map {}, Map {});
  }

}
