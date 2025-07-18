/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

package test.fixtures.refs;

import com.facebook.swift.codec.*;
import com.facebook.swift.codec.ThriftField.Requiredness;
import com.facebook.swift.codec.ThriftField.Recursiveness;
import com.google.common.collect.*;
import java.util.*;
import javax.annotation.Nullable;
import org.apache.thrift.*;
import org.apache.thrift.TException;
import org.apache.thrift.transport.*;
import org.apache.thrift.protocol.*;
import org.apache.thrift.protocol.TProtocol;
import static com.google.common.base.MoreObjects.toStringHelper;
import static com.google.common.base.MoreObjects.ToStringHelper;

@SwiftGenerated
@com.facebook.swift.codec.ThriftStruct(value="RecursiveStruct", builder=RecursiveStruct.Builder.class)
public final class RecursiveStruct implements com.facebook.thrift.payload.ThriftSerializable {
    @ThriftConstructor
    public RecursiveStruct(
        @com.facebook.swift.codec.ThriftField(value=1, name="mes", requiredness=Requiredness.OPTIONAL, isRecursive=Recursiveness.TRUE) final List<test.fixtures.refs.RecursiveStruct> mes
    ) {
        this.mes = mes;
    }
    
    @ThriftConstructor
    protected RecursiveStruct() {
      this.mes = null;
    }

    public static Builder builder() {
      return new Builder();
    }

    public static Builder builder(RecursiveStruct other) {
      return new Builder(other);
    }

    public static class Builder {
        private List<test.fixtures.refs.RecursiveStruct> mes = null;
    
        @com.facebook.swift.codec.ThriftField(value=1, name="mes", requiredness=Requiredness.OPTIONAL, isRecursive=Recursiveness.TRUE)    public Builder setMes(List<test.fixtures.refs.RecursiveStruct> mes) {
            this.mes = mes;
            return this;
        }
    
        public List<test.fixtures.refs.RecursiveStruct> getMes() { return mes; }
    
        public Builder() { }
        public Builder(RecursiveStruct other) {
            this.mes = other.mes;
        }
    
        @ThriftConstructor
        public RecursiveStruct build() {
            RecursiveStruct result = new RecursiveStruct (
                this.mes
            );
            return result;
        }
    }
    
    public static final Map<String, Integer> NAMES_TO_IDS = new HashMap<>();
    public static final Map<String, Integer> THRIFT_NAMES_TO_IDS = new HashMap<>();
    public static final Map<Integer, TField> FIELD_METADATA = new HashMap<>();
    private static final TStruct STRUCT_DESC = new TStruct("RecursiveStruct");
    private final List<test.fixtures.refs.RecursiveStruct> mes;
    public static final int _MES = 1;
    private static final TField MES_FIELD_DESC = new TField("mes", TType.LIST, (short)1);
    static {
      NAMES_TO_IDS.put("mes", 1);
      THRIFT_NAMES_TO_IDS.put("mes", 1);
      FIELD_METADATA.put(1, MES_FIELD_DESC);
    }
    
    @Nullable
    @com.facebook.swift.codec.ThriftField(value=1, name="mes", requiredness=Requiredness.OPTIONAL, isRecursive=Recursiveness.TRUE)
    public List<test.fixtures.refs.RecursiveStruct> getMes() { return mes; }

    @java.lang.Override
    public String toString() {
        ToStringHelper helper = toStringHelper(this);
        helper.add("mes", mes);
        return helper.toString();
    }

    @java.lang.Override
    public boolean equals(java.lang.Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
    
        RecursiveStruct other = (RecursiveStruct)o;
    
        return
            Objects.equals(mes, other.mes) &&
            true;
    }

    @java.lang.Override
    public int hashCode() {
        return Arrays.deepHashCode(new java.lang.Object[] {
            mes
        });
    }

    
    public static com.facebook.thrift.payload.Reader<RecursiveStruct> asReader() {
      return RecursiveStruct::read0;
    }
    
    public static RecursiveStruct read0(TProtocol oprot) throws TException {
      TField __field;
      oprot.readStructBegin(RecursiveStruct.NAMES_TO_IDS, RecursiveStruct.THRIFT_NAMES_TO_IDS, RecursiveStruct.FIELD_METADATA);
      RecursiveStruct.Builder builder = new RecursiveStruct.Builder();
      while (true) {
        __field = oprot.readFieldBegin();
        if (__field.type == TType.STOP) { break; }
        switch (__field.id) {
        case _MES:
          if (__field.type == TType.LIST) {
            List<test.fixtures.refs.RecursiveStruct> mes;
                {
                TList _list = oprot.readListBegin();
                mes = new ArrayList<test.fixtures.refs.RecursiveStruct>(Math.max(0, _list.size));
                for (int _i = 0; (_list.size < 0) ? oprot.peekList() : (_i < _list.size); _i++) {
                    
                    test.fixtures.refs.RecursiveStruct _value1 = test.fixtures.refs.RecursiveStruct.read0(oprot);
                    mes.add(_value1);
                }
                oprot.readListEnd();
                }
            builder.setMes(mes);
          } else {
            TProtocolUtil.skip(oprot, __field.type);
          }
          break;
        default:
          TProtocolUtil.skip(oprot, __field.type);
          break;
        }
        oprot.readFieldEnd();
      }
      oprot.readStructEnd();
      return builder.build();
    }

    public void write0(TProtocol oprot) throws TException {
      oprot.writeStructBegin(STRUCT_DESC);
      if (mes != null) {
        oprot.writeFieldBegin(MES_FIELD_DESC);
        List<test.fixtures.refs.RecursiveStruct>  _iter0 = mes;
        oprot.writeListBegin(new TList(TType.STRUCT, _iter0.size()));
            for (test.fixtures.refs.RecursiveStruct _iter1 : _iter0) {
              _iter1.write0(oprot);
            }
            oprot.writeListEnd();
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    private static class _RecursiveStructLazy {
        private static final RecursiveStruct _DEFAULT = new RecursiveStruct.Builder().build();
    }
    
    public static RecursiveStruct defaultInstance() {
        return  _RecursiveStructLazy._DEFAULT;
    }
}
