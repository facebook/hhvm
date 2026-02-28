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

final class ThriftTypeStructAdapter implements IThriftAdapter {
  const type THackType = this;
  const type TThriftType = apache_thrift_type_rep_TypeStruct;

  private function __construct(private self::TThriftType $typeStruct)[] {}

  public static function toThrift(
    this::THackType $hack_obj,
  )[write_props]: this::TThriftType {
    return $hack_obj->typeStruct;
  }

  public static function fromThrift(
    this::TThriftType $thrift_obj,
  )[]: this::THackType {
    return new self($thrift_obj);
  }
  public function getTypeURI()[]: ?apache_thrift_type_standard_TypeUri {
    $name = $this->typeStruct->name;
    if ($name is null) {
      return null;
    }
    switch ($name->getType()) {
      case apache_thrift_type_standard_TypeNameEnum::structType:
        return $name->getx_structType();
      case apache_thrift_type_standard_TypeNameEnum::unionType:
        return $name->getx_unionType();
      case apache_thrift_type_standard_TypeNameEnum::exceptionType:
        return $name->getx_exceptionType();
      case apache_thrift_type_standard_TypeNameEnum::enumType:
        return $name->getx_enumType();
      case apache_thrift_type_standard_TypeNameEnum::typedefType:
        return $name->getx_typedefType();
      default:
        return null;
    }
  }

  public function toThriftStruct()[]: this::TThriftType {
    return $this->typeStruct;
  }

  public function toTypeSpec()[]: ThriftStructTypes::TGenericSpec {
    $type_struct = $this->typeStruct;
    $type_spec = shape('type' => TType::VOID);
    $type_name = $type_struct->name as nonnull;
    switch ($type_name->getType()) {
      case apache_thrift_type_standard_TypeNameEnum::boolType:
        $type_spec['type'] = TType::BOOL;
        break;
      case apache_thrift_type_standard_TypeNameEnum::byteType:
        $type_spec['type'] = TType::BYTE;
        break;
      case apache_thrift_type_standard_TypeNameEnum::i16Type:
        $type_spec['type'] = TType::I16;
        break;
      case apache_thrift_type_standard_TypeNameEnum::i32Type:
        $type_spec['type'] = TType::I32;
        break;
      case apache_thrift_type_standard_TypeNameEnum::i64Type:
        $type_spec['type'] = TType::I64;
        break;
      case apache_thrift_type_standard_TypeNameEnum::floatType:
        $type_spec['type'] = TType::FLOAT;
        break;
      case apache_thrift_type_standard_TypeNameEnum::doubleType:
        $type_spec['type'] = TType::DOUBLE;
        break;
      case apache_thrift_type_standard_TypeNameEnum::stringType:
        $type_spec['type'] = TType::STRING;
        break;
      case apache_thrift_type_standard_TypeNameEnum::binaryType:
        $type_spec['type'] = TType::STRING;
        $type_spec['is_binary'] = true;
        break;
      case apache_thrift_type_standard_TypeNameEnum::unionType:
        $type_spec['union'] = true;
        $type_spec['type'] = TType::STRUCT;
        $type_spec['class'] = ThriftTypeRegistry::getxHackTypeForTypeUri(
          $type_name->getx_unionType(),
        );
        break;
      case apache_thrift_type_standard_TypeNameEnum::structType:
        $type_spec['type'] = TType::STRUCT;
        $type_spec['class'] = ThriftTypeRegistry::getxHackTypeForTypeUri(
          $type_name->getx_structType(),
        );
        break;
      case apache_thrift_type_standard_TypeNameEnum::exceptionType:
        $type_spec['type'] = TType::STRUCT;
        $type_spec['class'] = ThriftTypeRegistry::getxHackTypeForTypeUri(
          $type_name->getx_exceptionType(),
        );
        break;
      case apache_thrift_type_standard_TypeNameEnum::enumType:
        $type_spec['type'] = TType::I32;
        $type_spec['enum'] = ThriftTypeRegistry::getxHackTypeForTypeUri(
          $type_name->getx_enumType(),
        );
        break;
      case apache_thrift_type_standard_TypeNameEnum::listType:
        $type_spec['type'] = TType::LST;
        $type_spec['elem'] =
          self::fromThrift($type_struct->params[0])->toTypeSpec();
        $type_spec['etype'] = $type_spec['elem']['type'];
        $type_spec['format'] = 'harray';
        break;
      case apache_thrift_type_standard_TypeNameEnum::setType:
        $type_spec['type'] = TType::SET;
        $type_spec['elem'] =
          self::fromThrift($type_struct->params[0])->toTypeSpec();
        $type_spec['etype'] = $type_spec['elem']['type'];
        $type_spec['format'] = 'harray';
        break;
      case apache_thrift_type_standard_TypeNameEnum::mapType:
        $type_spec['type'] = TType::MAP;

        $type_spec['key'] =
          self::fromThrift($type_struct->params[0])->toTypeSpec();
        $type_spec['ktype'] = $type_spec['key']['type'];
        $type_spec['val'] =
          self::fromThrift($type_struct->params[1])->toTypeSpec();
        $type_spec['vtype'] = $type_spec['val']['type'];
        $type_spec['format'] = 'harray';
        break;
      case apache_thrift_type_standard_TypeNameEnum::typedefType:
        throw new TApplicationException(
          "typedefType is not yet supported, directly provide the underlying type",
        );
      case apache_thrift_type_standard_TypeNameEnum::_EMPTY_:
        throw new TApplicationException("No type information available");
    }
    return HH\FIXME\UNSAFE_CAST<shape(...), ThriftStructTypes::TGenericSpec>(
      $type_spec,
    );
  }

  public static function fromTypeSpec(
    ThriftStructTypes::TGenericSpec $hack_obj,
  )[write_props]: this {
    $type = apache_thrift_type_rep_TypeStruct::withDefaultValues();
    $name = apache_thrift_type_standard_TypeName::withDefaultValues();
    $params = vec[];
    switch ($hack_obj['type']) {
      case TType::BOOL:
        $name->set_boolType(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::BYTE:
        $name->set_byteType(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::I16:
        $name->set_i16Type(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::I32:
        if (Shapes::idx($hack_obj, 'enum') === null) {
          $name->set_i32Type(apache_thrift_type_standard_Void::Unused);
        } else {
          $type_uri = apache_thrift_type_standard_TypeUri::fromShape(shape(
            'uri' => ThriftTypeRegistry::getxCanonicalUriForHackType(
              ArgAssert::isAnyClassname($hack_obj['enum']),
            ),
          ));
          $name->set_enumType($type_uri);
        }
        break;
      case TType::I64:
        $name->set_i64Type(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::DOUBLE:
        $name->set_doubleType(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::FLOAT:
        $name->set_floatType(apache_thrift_type_standard_Void::Unused);
        break;
      case TType::STRING:
        if (Shapes::idx($hack_obj, 'is_binary', false)) {
          $name->set_binaryType(apache_thrift_type_standard_Void::Unused);
        } else {
          $name->set_stringType(apache_thrift_type_standard_Void::Unused);
        }
        break;
      case TType::STRUCT:
        $type_uri = apache_thrift_type_standard_TypeUri::withDefaultValues();
        $class_nm = Shapes::at($hack_obj, 'class');
        $type_uri->set_uri(
          ThriftTypeRegistry::getxCanonicalUriForHackType($class_nm),
        );

        if (Shapes::idx($hack_obj, 'union', false)) {
          $name->set_unionType($type_uri);
        } else {
          $class = new ReflectionClass($class_nm);
          if ($class->isSubclassOf(TException::class)) {

            $name->set_exceptionType($type_uri);
          } else {
            $name->set_structType($type_uri);
          }
        }
        break;
      case TType::LST:
        $name->set_listType(apache_thrift_type_standard_Void::Unused);
        $params[] =
          self::toThrift(self::fromTypeSpec(Shapes::at($hack_obj, 'elem')));
        break;

      case TType::MAP:
        $name->set_mapType(apache_thrift_type_standard_Void::Unused);
        $params[] =
          self::toThrift(self::fromTypeSpec(Shapes::at($hack_obj, 'key')));
        $params[] =
          self::toThrift(self::fromTypeSpec(Shapes::at($hack_obj, 'val')));
        break;
      case TType::SET:
        $name->set_setType(apache_thrift_type_standard_Void::Unused);
        $params[] =
          self::toThrift(self::fromTypeSpec(Shapes::at($hack_obj, 'elem')));
        break;
      case TType::STOP:
      case TType::UTF16:
      case TType::UTF8:
      case TType::VOID:
        throw new TApplicationException(
          "Type ".
          TType::getNames()[$hack_obj['type']].
          " cannot be serialized in thrift/any.",
        );
    }
    invariant(
      $name->getType() !== apache_thrift_type_standard_TypeNameEnum::_EMPTY_,
      "Type name should have at least one value set",
    );
    $type->name = $name;
    $type->params = $params;

    return new self($type);
  }

  public static function fromHackType<reify T>()[write_props]: this {
    $ts = HH\ReifiedGenerics\get_type_structure<T>();
    $type_struct = self::getTypeStructFromHackTypeStructure($ts);
    return self::fromThrift($type_struct);
  }

  /**
   * Infers a Thrift type from the corresponding native Hack type.
   * Because certain types are not 1-1 (e.g. Hack ints can correspond to any
   * integral Thrift types), we use the most permissive Thrift type (e.g. i64).
   */
  private static function getTypeStructFromHackTypeStructure<T>(
    TypeStructure<T> $ts,
  )[write_props]: apache_thrift_type_rep_TypeStruct {
    $type_name = apache_thrift_type_standard_TypeName::withDefaultValues();
    $type_structure = apache_thrift_type_rep_TypeStruct::fromShape(shape(
      'name' => $type_name,
    ));

    switch ($ts['kind']) {
      case TypeStructureKind::OF_INT:
        $type_name->set_i64Type(apache_thrift_type_standard_Void::Unused);
        break;
      case TypeStructureKind::OF_BOOL:
        $type_name->set_boolType(apache_thrift_type_standard_Void::Unused);
        break;
      case TypeStructureKind::OF_FLOAT:
        $type_name->set_floatType(apache_thrift_type_standard_Void::Unused);
        break;
      case TypeStructureKind::OF_CLASS_PTR:
      case TypeStructureKind::OF_STRING:
        $type_name->set_stringType(apache_thrift_type_standard_Void::Unused);
        break;

      // Structs, unions, exceptions, enums
      case TypeStructureKind::OF_CLASS:
      case TypeStructureKind::OF_ENUM:
        $cls = Shapes::at($ts, 'classname') as nonnull;
        $type_uri = apache_thrift_type_standard_TypeUri::fromShape(shape(
          'uri' => ThriftTypeRegistry::getxCanonicalUriForHackType($cls),
        ));

        if (Classnames::is($cls, IThriftUnion::class)) {
          $type_name->set_unionType($type_uri);
        } else if (Classnames::is($cls, TException::class)) {
          $type_name->set_exceptionType($type_uri);
        } else if (Classnames::is($cls, IThriftStruct::class)) {
          $type_name->set_structType($type_uri);
        } else {
          $type_name->set_enumType($type_uri);
        }
        break;
      case TypeStructureKind::OF_DICT:
        $type_name->set_mapType(apache_thrift_type_standard_Void::Unused);
        break;
      case TypeStructureKind::OF_VEC:
        $type_name->set_listType(apache_thrift_type_standard_Void::Unused);
        break;
      case TypeStructureKind::OF_KEYSET:
        $type_name->set_setType(apache_thrift_type_standard_Void::Unused);
        break;
      default:
        throw new TApplicationException(
          "Unhandled kind when inferring Thrift type from Hack type structure.",
        );
    }

    $generic_types = Shapes::idx($ts, 'generic_types');
    if ($generic_types is nonnull) {
      $type_structure->params = Vec\map(
        HH\FIXME\UNSAFE_CAST<vec<mixed>, vec<TypeStructure<mixed>>>(
          $generic_types,
        ),
        self::getTypeStructFromHackTypeStructure<>,
      );
    }

    return $type_structure;
  }
}
