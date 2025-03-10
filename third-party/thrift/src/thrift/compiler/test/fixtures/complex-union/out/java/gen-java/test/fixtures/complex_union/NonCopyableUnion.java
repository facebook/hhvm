/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

package test.fixtures.complex_union;

import com.facebook.swift.codec.*;
import com.facebook.swift.codec.ThriftField.Requiredness;
import com.facebook.swift.codec.ThriftField.Recursiveness;
import java.util.*;
import org.apache.thrift.*;
import org.apache.thrift.transport.*;
import org.apache.thrift.protocol.*;

import static com.google.common.base.MoreObjects.toStringHelper;

@SwiftGenerated
@ThriftUnion("NonCopyableUnion")
public final class NonCopyableUnion implements com.facebook.thrift.payload.ThriftSerializable {
    
    private static final boolean allowNullFieldValues =
        System.getProperty("thrift.union.allow-null-field-values", "false").equalsIgnoreCase("true");

    private static final TStruct STRUCT_DESC = new TStruct("NonCopyableUnion");
    private static final Map<String, Integer> NAMES_TO_IDS = new HashMap();
    public static final Map<String, Integer> THRIFT_NAMES_TO_IDS = new HashMap();
    private static final Map<Integer, TField> FIELD_METADATA = new HashMap<>();
    private static final NonCopyableUnion _DEFAULT = new NonCopyableUnion();

    public static final int _S = 1;
    private static final TField S_FIELD_DESC = new TField("s", TType.STRUCT, (short)1);

    static {
      NAMES_TO_IDS.put("s", 1);
      THRIFT_NAMES_TO_IDS.put("s", 1);
      FIELD_METADATA.put(1, S_FIELD_DESC);
    }

    private java.lang.Object value;
    private short id;

    public enum TypeEnum {
      __FBTHRIFT_EMPTY_UNION__,
      S,
    }

    public static NonCopyableUnion from(int _id, java.lang.Object _field) {
        return from((short) _id, _field);
    }

    public static NonCopyableUnion from(short _id, java.lang.Object _field) {
        java.util.Objects.requireNonNull(_field);
        if (!FIELD_METADATA.containsKey(Integer.valueOf(_id))) {
            throw new java.lang.IllegalArgumentException("unknown field " + _id);
        }

        NonCopyableUnion _u = new  NonCopyableUnion();

        try {
            switch(_id) {
                case 1:
                    _u.id = _id;
                    _u.value = (test.fixtures.complex_union.NonCopyableStruct) _field;
                    return _u;
                default:
                throw new IllegalArgumentException("invalid type " + _field.getClass().getName() + " for field " + _id);
            }
        } catch (java.lang.Exception t) {
            throw new IllegalArgumentException("invalid type " + _field.getClass().getName() + " for field " + _id);
        }
    }

    @ThriftConstructor
    public NonCopyableUnion() {
    }
    
    @ThriftConstructor
    @Deprecated
    public NonCopyableUnion(final test.fixtures.complex_union.NonCopyableStruct s) {
        if (!NonCopyableUnion.allowNullFieldValues && s == null) {
            throw new TProtocolException("Cannot initialize Union field 'NonCopyableUnion.s' with null value!");
        }
        this.value = s;
        this.id = 1;
    }
    
    public static NonCopyableUnion fromS(final test.fixtures.complex_union.NonCopyableStruct s) {
        NonCopyableUnion res = new NonCopyableUnion();
        if (!NonCopyableUnion.allowNullFieldValues && s == null) {
            throw new TProtocolException("Cannot initialize Union field 'NonCopyableUnion.s' with null value!");
        }
        res.value = s;
        res.id = 1;
        return res;
    }

    

    @com.facebook.swift.codec.ThriftField(value=1, name="s", requiredness=Requiredness.NONE)
    public test.fixtures.complex_union.NonCopyableStruct getS() {
        if (this.id != 1) {
            throw new IllegalStateException("Not a s element!");
        }
        return (test.fixtures.complex_union.NonCopyableStruct) value;
    }

    public boolean isSetS() {
        return this.id == 1;
    }

    @ThriftUnionId
    public short getThriftId() {
        return this.id;
    }

    public TypeEnum getThriftUnionType() {
      switch(this.id) {
        case 0:
          return TypeEnum.__FBTHRIFT_EMPTY_UNION__;
        case 1:
          return TypeEnum.S;
        default:
          throw new IllegalStateException("unreachable");
      }
    }

    public String getThriftName() {
        TField tField = (TField) FIELD_METADATA.get((int) this.id);
        if (tField == null) {
            return "null";
        } else {
            return tField.name;
        }
    }

    public <T> T accept(Visitor<T> visitor) {
        if (isSetS()) {
            return visitor.visitS(getS());
        }

        throw new IllegalStateException("Visitor missing for type " + this.getThriftUnionType());
    }

    @java.lang.Override
    public String toString() {
        return toStringHelper(this)
            .add("value", value)
            .add("id", id)
            .add("name", getThriftName())
            .add("type", value == null ? "<null>" : value.getClass().getSimpleName())
            .toString();
    }

    @java.lang.Override
    public boolean equals(java.lang.Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }

        NonCopyableUnion other = (NonCopyableUnion)o;

        return Objects.equals(this.id, other.id)
                && Objects.deepEquals(this.value, other.value);
    }

    @java.lang.Override
    public int hashCode() {
        return Arrays.deepHashCode(new java.lang.Object[] {
            id,
            value,
        });
    }

    public interface Visitor<T> {
        default T visit(NonCopyableUnion acceptor) {
        return acceptor.accept(this);
        }

        T visitS(test.fixtures.complex_union.NonCopyableStruct s);
    }

    public void write0(TProtocol oprot) throws TException {
      if (this.id != 0 && this.value == null ){
        if(allowNullFieldValues) {
          // Warning: this path will generate corrupt serialized data!
          return;
        } else {
          throw new TProtocolException("Cannot write a Union with marked-as-set but null value!");
        }
      }
      oprot.writeStructBegin(STRUCT_DESC);
      switch (this.id) {
      case _S: {
        oprot.writeFieldBegin(S_FIELD_DESC);
        test.fixtures.complex_union.NonCopyableStruct s = (test.fixtures.complex_union.NonCopyableStruct)this.value;
        s.write0(oprot);
        oprot.writeFieldEnd();
        break;
      }
      default:
          // ignore unknown field
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    
    public static com.facebook.thrift.payload.Reader<NonCopyableUnion> asReader() {
      return NonCopyableUnion::read0;
    }
    
    public static NonCopyableUnion read0(TProtocol oprot) throws TException {
      NonCopyableUnion res = new NonCopyableUnion();
      res.value = null;
      res.id = (short) 0;
      oprot.readStructBegin(NonCopyableUnion.NAMES_TO_IDS, NonCopyableUnion.THRIFT_NAMES_TO_IDS, NonCopyableUnion.FIELD_METADATA);
      TField __field = oprot.readFieldBegin();
      if (__field.type != TType.STOP) {
          switch (__field.id) {
          case _S:
            if (__field.type == S_FIELD_DESC.type) {
              test.fixtures.complex_union.NonCopyableStruct s = test.fixtures.complex_union.NonCopyableStruct.read0(oprot);
              res.value = s;
            }
            break;
          default:
            TProtocolUtil.skip(oprot, __field.type);
          }
        if (res.value != null) {
          res.id = __field.id;
        }
        oprot.readFieldEnd();
        TField __stopField = oprot.readFieldBegin(); // Consume the STOP byte
        if (__stopField.type != TType.STOP) {
          throw new TProtocolException(TProtocolException.INVALID_DATA, "Union 'NonCopyableUnion' is missing a STOP byte");
        }
      }
      oprot.readStructEnd();
      return res;
    }
    public static NonCopyableUnion defaultInstance() {
        return _DEFAULT;
    }

}
