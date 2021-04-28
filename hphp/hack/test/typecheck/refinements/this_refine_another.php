<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface INeedsDependencyTrackingMeerkatStep {
  require extends BaseTypeLevelMeerkatStep;

  /**
   * Return a list of files that, if changed, would require reprocessing the
   * passed in type.
   */
  protected function genDependentFilesForType(
    this::TType $_type,
  ): keyset<string>;
}

abstract class BaseTypeLevelMeerkatStep /* extends MeerkatStep */ {
  abstract const type TCategory as int;
    const type TType = shape(
          'category' => this::TCategory,
    'type_name' => string,
    );
}

abstract class TypeLevelMeerkatStep extends BaseTypeLevelMeerkatStep {
}

trait NeedsDependencyTrackingMeerkatStepTrait {
  require implements INeedsDependencyTrackingMeerkatStep;

  final public function genUpdateDependencyArtifact(
    this::TCategory $category,
    (this & TypeLevelMeerkatStep) $a,
  ): void {
    $a->genDependentFilesForType(
        shape('category' => $category, 'type_name' => 'a'),
    );
  }
}
