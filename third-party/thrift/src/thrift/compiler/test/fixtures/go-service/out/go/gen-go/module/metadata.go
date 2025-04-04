// Autogenerated by Thrift for thrift/compiler/test/fixtures/go-service/src/module.thrift
//
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
//  @generated

package module

import (
    "maps"

    thrift "github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
    metadata "github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// (needed to ensure safety because of naive import list construction)
var _ = thrift.VOID
var _ = maps.Copy[map[int]int, map[int]int]
var _ = metadata.GoUnusedProtection__

// Premade Thrift types
var (
    premadeThriftType_string = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_STRING_TYPE),
        )
    }()
    premadeThriftType_module_GetEntityRequest = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTStruct(
            metadata.NewThriftStructType().
                SetName("module.GetEntityRequest"),
        )
    }()
    premadeThriftType_module_GetEntityResponse = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTStruct(
            metadata.NewThriftStructType().
                SetName("module.GetEntityResponse"),
        )
    }()
    premadeThriftType_list_string = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTList(
            metadata.NewThriftListType().
                SetValueType(premadeThriftType_string),
        )
    }()
    premadeThriftType_module_NonComparableStruct = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTStruct(
            metadata.NewThriftStructType().
                SetName("module.NonComparableStruct"),
        )
    }()
    premadeThriftType_i64 = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_I64_TYPE),
        )
    }()
    premadeThriftType_map_module_NonComparableStruct_i64 = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTMap(
            metadata.NewThriftMapType().
                SetKeyType(premadeThriftType_module_NonComparableStruct).
                SetValueType(premadeThriftType_i64),
        )
    }()
    premadeThriftType_bool = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_BOOL_TYPE),
        )
    }()
    premadeThriftType_byte = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_BYTE_TYPE),
        )
    }()
    premadeThriftType_i16 = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_I16_TYPE),
        )
    }()
    premadeThriftType_i32 = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_I32_TYPE),
        )
    }()
    premadeThriftType_double = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_DOUBLE_TYPE),
        )
    }()
    premadeThriftType_binary = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTPrimitive(
            thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_BINARY_TYPE),
        )
    }()
    premadeThriftType_map_string_string = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTMap(
            metadata.NewThriftMapType().
                SetKeyType(premadeThriftType_string).
                SetValueType(premadeThriftType_string),
        )
    }()
    premadeThriftType_set_string = func() *metadata.ThriftType {
        return metadata.NewThriftType().SetTSet(
            metadata.NewThriftSetType().
                SetValueType(premadeThriftType_string),
        )
    }()
)

// Helper type to allow us to store Thrift types in a slice at compile time,
// and put them in a map at runtime. See comment at the top of template
// about a compilation limitation that affects map literals.
type thriftTypeWithFullName struct {
    fullName   string
    thriftType *metadata.ThriftType
}

var premadeThriftTypesMap = func() map[string]*metadata.ThriftType {
    thriftTypesWithFullName := make([]thriftTypeWithFullName, 0)
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "string", premadeThriftType_string })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "module.GetEntityRequest", premadeThriftType_module_GetEntityRequest })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "module.GetEntityResponse", premadeThriftType_module_GetEntityResponse })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "module.NonComparableStruct", premadeThriftType_module_NonComparableStruct })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "i64", premadeThriftType_i64 })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "bool", premadeThriftType_bool })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "byte", premadeThriftType_byte })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "i16", premadeThriftType_i16 })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "i32", premadeThriftType_i32 })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "double", premadeThriftType_double })
    thriftTypesWithFullName = append(thriftTypesWithFullName, thriftTypeWithFullName{ "binary", premadeThriftType_binary })

    fbthriftThriftTypesMap := make(map[string]*metadata.ThriftType, len(thriftTypesWithFullName))
    for _, value := range thriftTypesWithFullName {
        fbthriftThriftTypesMap[value.fullName] = value.thriftType
    }
    return fbthriftThriftTypesMap
}()

var structMetadatas = func() []*metadata.ThriftStruct {
    fbthriftResults := make([]*metadata.ThriftStruct, 0)
    for _, fbthriftStructSpec := range premadeStructSpecs {
        if !fbthriftStructSpec.IsException {
            fbthriftResults = append(fbthriftResults, getMetadataThriftStruct(fbthriftStructSpec))
        }
    }
    return fbthriftResults
}()

var exceptionMetadatas = func() []*metadata.ThriftException {
    fbthriftResults := make([]*metadata.ThriftException, 0)
    for _, fbthriftStructSpec := range premadeStructSpecs {
        if fbthriftStructSpec.IsException {
            fbthriftResults = append(fbthriftResults, getMetadataThriftException(fbthriftStructSpec))
        }
    }
    return fbthriftResults
}()

var enumMetadatas = func() []*metadata.ThriftEnum {
    fbthriftResults := make([]*metadata.ThriftEnum, 0)
    return fbthriftResults
}()

var serviceMetadatas = func() []*metadata.ThriftService {
    fbthriftResults := make([]*metadata.ThriftService, 0)
    fbthriftResults = append(fbthriftResults, metadata.NewThriftService().
    SetName("module.GetEntity").
    SetFunctions(
        []*metadata.ThriftFunction{
            metadata.NewThriftFunction().
    SetName("getEntity").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_module_GetEntityResponse).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("r").
    SetIsOptional(false).
    SetType(premadeThriftType_module_GetEntityRequest),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getBool").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_bool),
            metadata.NewThriftFunction().
    SetName("getByte").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_byte),
            metadata.NewThriftFunction().
    SetName("getI16").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i16),
            metadata.NewThriftFunction().
    SetName("getI32").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32),
            metadata.NewThriftFunction().
    SetName("getI64").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i64),
            metadata.NewThriftFunction().
    SetName("getDouble").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_double),
            metadata.NewThriftFunction().
    SetName("getString").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_string),
            metadata.NewThriftFunction().
    SetName("getBinary").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_binary),
            metadata.NewThriftFunction().
    SetName("getMap").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_map_string_string),
            metadata.NewThriftFunction().
    SetName("getSet").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_set_string),
            metadata.NewThriftFunction().
    SetName("getList").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_list_string),
            metadata.NewThriftFunction().
    SetName("getLegacyStuff").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("numPos").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(-1).
    SetName("numNeg1").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(-2).
    SetName("numNeg2").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getCtxCollision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("ctx").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getCtx1Collision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("ctx").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(2).
    SetName("ctx1").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getContextCollision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("context").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getOutCollision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("out").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getOut1Collision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("out").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(2).
    SetName("out1").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getInCollision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("in").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getIn1Collision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("in").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(2).
    SetName("in1").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getErrCollision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("err").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
            metadata.NewThriftFunction().
    SetName("getErr1Collision").
    SetIsOneway(false).
    SetReturnType(premadeThriftType_i32).
    SetArguments(
        []*metadata.ThriftField{
            metadata.NewThriftField().
    SetId(1).
    SetName("err").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
            metadata.NewThriftField().
    SetId(2).
    SetName("err1").
    SetIsOptional(false).
    SetType(premadeThriftType_i64),
        },
    ),
        },
    ))
    return fbthriftResults
}()

// GetMetadataThriftType (INTERNAL USE ONLY).
// Returns metadata ThriftType for a given full type name.
func GetMetadataThriftType(fullName string) *metadata.ThriftType {
    return premadeThriftTypesMap[fullName]
}

// GetThriftMetadata returns complete Thrift metadata for current and imported packages.
func GetThriftMetadata() *metadata.ThriftMetadata {
    allEnumsMap := make(map[string]*metadata.ThriftEnum)
    allStructsMap := make(map[string]*metadata.ThriftStruct)
    allExceptionsMap := make(map[string]*metadata.ThriftException)
    allServicesMap := make(map[string]*metadata.ThriftService)

    // Add enum metadatas from the current program...
    for _, enumMetadata := range enumMetadatas {
        allEnumsMap[enumMetadata.GetName()] = enumMetadata
    }
    // Add struct metadatas from the current program...
    for _, structMetadata := range structMetadatas {
        allStructsMap[structMetadata.GetName()] = structMetadata
    }
    // Add exception metadatas from the current program...
    for _, exceptionMetadata := range exceptionMetadatas {
        allExceptionsMap[exceptionMetadata.GetName()] = exceptionMetadata
    }
    // Add service metadatas from the current program...
    for _, serviceMetadata := range serviceMetadatas {
        allServicesMap[serviceMetadata.GetName()] = serviceMetadata
    }

    // Obtain Thrift metadatas from recursively included programs...
    var recursiveThriftMetadatas []*metadata.ThriftMetadata

    // ...now merge metadatas from recursively included programs.
    for _, thriftMetadata := range recursiveThriftMetadatas {
        maps.Copy(allEnumsMap, thriftMetadata.GetEnums())
        maps.Copy(allStructsMap, thriftMetadata.GetStructs())
        maps.Copy(allExceptionsMap, thriftMetadata.GetExceptions())
        maps.Copy(allServicesMap, thriftMetadata.GetServices())
    }

    return metadata.NewThriftMetadata().
        SetEnums(allEnumsMap).
        SetStructs(allStructsMap).
        SetExceptions(allExceptionsMap).
        SetServices(allServicesMap)
}

// GetThriftMetadataForService returns Thrift metadata for the given service.
func GetThriftMetadataForService(scopedServiceName string) *metadata.ThriftMetadata {
    thriftMetadata := GetThriftMetadata()

    allServicesMap := thriftMetadata.GetServices()
    relevantServicesMap := make(map[string]*metadata.ThriftService)

    serviceMetadata := allServicesMap[scopedServiceName]
    // Visit and record all recursive parents of the target service.
    for serviceMetadata != nil {
        relevantServicesMap[serviceMetadata.GetName()] = serviceMetadata
        if serviceMetadata.IsSetParent() {
            serviceMetadata = allServicesMap[serviceMetadata.GetParent()]
        } else {
            serviceMetadata = nil
        }
    }

    thriftMetadata.SetServices(relevantServicesMap)

    return thriftMetadata
}

func getMetadataThriftPrimitiveType(s *thrift.CodecPrimitiveSpec) *metadata.ThriftPrimitiveType {
	var value metadata.ThriftPrimitiveType

	switch s.PrimitiveType {
	case thrift.CODEC_PRIMITIVE_TYPE_BYTE:
		value = metadata.ThriftPrimitiveType_THRIFT_BYTE_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_BOOL:
		value = metadata.ThriftPrimitiveType_THRIFT_BOOL_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_I16:
		value = metadata.ThriftPrimitiveType_THRIFT_I16_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_I32:
		value = metadata.ThriftPrimitiveType_THRIFT_I32_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_I64:
		value = metadata.ThriftPrimitiveType_THRIFT_I64_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_FLOAT:
		value = metadata.ThriftPrimitiveType_THRIFT_FLOAT_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_DOUBLE:
		value = metadata.ThriftPrimitiveType_THRIFT_DOUBLE_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_BINARY:
		value = metadata.ThriftPrimitiveType_THRIFT_BINARY_TYPE
	case thrift.CODEC_PRIMITIVE_TYPE_STRING:
		value = metadata.ThriftPrimitiveType_THRIFT_STRING_TYPE
	}

	return thrift.Pointerize(value)
}

func getMetadataThriftEnumType(s *thrift.CodecEnumSpec) *metadata.ThriftEnumType {
	return metadata.NewThriftEnumType().
		SetName(s.ScopedName)
}

func getMetadataThriftSetType(s *thrift.CodecSetSpec) *metadata.ThriftSetType {
	return metadata.NewThriftSetType().
		SetValueType(getMetadataThriftType(s.ElementTypeSpec))
}

func getMetadataThriftListType(s *thrift.CodecListSpec) *metadata.ThriftListType {
	return metadata.NewThriftListType().
		SetValueType(getMetadataThriftType(s.ElementTypeSpec))
}

func getMetadataThriftMapType(s *thrift.CodecMapSpec) *metadata.ThriftMapType {
	return metadata.NewThriftMapType().
		SetKeyType(getMetadataThriftType(s.KeyTypeSpec)).
		SetValueType(getMetadataThriftType(s.ValueTypeSpec))
}

func getMetadataThriftTypedefType(s *thrift.CodecTypedefSpec) *metadata.ThriftTypedefType {
	return metadata.NewThriftTypedefType().
		SetName(s.ScopedName).
		SetUnderlyingType(getMetadataThriftType(s.UnderlyingTypeSpec))
}

func getMetadataThriftStructType(s *thrift.CodecStructSpec) *metadata.ThriftStructType {
	return metadata.NewThriftStructType().
		SetName(s.ScopedName)
}

func getMetadataThriftUnionType(s *thrift.CodecStructSpec) *metadata.ThriftUnionType {
	return metadata.NewThriftUnionType().
		SetName(s.ScopedName)
}

func getMetadataThriftType(s *thrift.TypeSpec) *metadata.ThriftType {
	thriftType := metadata.NewThriftType()
	switch {
	case s.CodecPrimitiveSpec != nil:
		thriftType.SetTPrimitive(getMetadataThriftPrimitiveType(s.CodecPrimitiveSpec))
	case s.CodecEnumSpec != nil:
		thriftType.SetTEnum(getMetadataThriftEnumType(s.CodecEnumSpec))
	case s.CodecSetSpec != nil:
		thriftType.SetTSet(getMetadataThriftSetType(s.CodecSetSpec))
	case s.CodecListSpec != nil:
		thriftType.SetTList(getMetadataThriftListType(s.CodecListSpec))
	case s.CodecMapSpec != nil:
		thriftType.SetTMap(getMetadataThriftMapType(s.CodecMapSpec))
	case s.CodecTypedefSpec != nil:
		thriftType.SetTTypedef(getMetadataThriftTypedefType(s.CodecTypedefSpec))
	case s.CodecStructSpec != nil:
		if s.CodecStructSpec.IsUnion {
			thriftType.SetTUnion(getMetadataThriftUnionType(s.CodecStructSpec))
		} else {
			thriftType.SetTStruct(getMetadataThriftStructType(s.CodecStructSpec))
		}
	}
	return thriftType
}

func getMetadataThriftField(s *thrift.FieldSpec) *metadata.ThriftField {
	return metadata.NewThriftField().
		SetId(int32(s.ID)).
		SetName(s.Name).
		SetIsOptional(s.IsOptional).
		SetType(getMetadataThriftType(s.ValueTypeSpec))
}

func getMetadataThriftStruct(s *thrift.StructSpec) *metadata.ThriftStruct {
	metadataThriftFields := make([]*metadata.ThriftField, len(s.FieldSpecs), len(s.FieldSpecs))
	for i, fieldSpec := range s.FieldSpecs {
		metadataThriftFields[i] = getMetadataThriftField(&fieldSpec)
	}

	return metadata.NewThriftStruct().
		SetName(s.ScopedName).
		SetIsUnion(s.IsUnion).
		SetFields(metadataThriftFields)
}

func getMetadataThriftException(s *thrift.StructSpec) *metadata.ThriftException {
	metadataThriftFields := make([]*metadata.ThriftField, len(s.FieldSpecs), len(s.FieldSpecs))
	for i, fieldSpec := range s.FieldSpecs {
		metadataThriftFields[i] = getMetadataThriftField(&fieldSpec)
	}

	return metadata.NewThriftException().
		SetName(s.ScopedName).
		SetFields(metadataThriftFields)
}
