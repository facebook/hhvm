/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package com.facebook.thrift.annotation.rust_deprecated;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Collections;
import java.util.BitSet;
import java.util.Arrays;
import com.facebook.thrift.*;
import com.facebook.thrift.annotations.*;
import com.facebook.thrift.async.*;
import com.facebook.thrift.meta_data.*;
import com.facebook.thrift.server.*;
import com.facebook.thrift.transport.*;
import com.facebook.thrift.protocol.*;

/**
 * If `anyhow_to_application_exn` is true, this allows returning an
 * `anyhow::Error` from within a Thrift server handler method, and that
 * `anyhow::Error` will be turned into an `ApplicationException`. This is
 * similar in behavior to what happens if you throw an unhandled exception type
 * in Python Thrift server or C++ Thrift server.
 * 
 * The `ApplicationException` returned will have the error code `Unknown` and
 * message `format!("{:#}", anyhow_err)`.
 * 
 * NOTE: it is generally considered bad practice to use this because it
 * eliminates the ability to match on specific error types on the client side.
 * When possible, it is recommended you always return structured error types
 * (though it is more verbose). This annotation is provided solely for
 * convenience and should not be used services where error type matching is
 * needed.
 * 
 * Example, the following Thrift:
 * 
 * ```
 * service Foo {
 *   @rust.ServiceExn{anyhow_to_application_exn = true}
 *   void bar();
 * }
 * ```
 * 
 * would allow for the following Rust server impl:
 * 
 * ```
 * #[async_trait]
 * impl Foo for FooServerImpl {
 *     async fn bar() -> Result<(), BarExn> {
 *         if some_condition {
 *             Err(anyhow::anyhow!("some error"))?
 *         }
 * 
 *         // Note you must always convert to `anyhow::Error` first for non-`anyhow::Error`
 *         // types.
 *         some_client_call.await.context("failed some_client_call")?;
 * 
 *         // you can still return a structured exn type if desired
 *         return Err(BarExn::ie(...));
 *     }
 * }
 * ```
 * 
 * You can also use this annotation on the service definition itself to have it
 * apply to all methods on the service, e.g.
 * 
 * ```
 * @rust.ServiceExn{anyhow_to_application_exn = true}
 * service Foo {
 *   // Both `bar` and `baz` will support `anyhow::Error` -> `ApplicationException`.
 *   void bar();
 *   void baz();
 * }
 * ```
 */
@SuppressWarnings({ "unused", "serial" })
public class ServiceExn implements TBase, java.io.Serializable, Cloneable, Comparable<ServiceExn> {
  private static final TStruct STRUCT_DESC = new TStruct("ServiceExn");
  private static final TField ANYHOW_TO_APPLICATION_EXN_FIELD_DESC = new TField("anyhow_to_application_exn", TType.BOOL, (short)1);

  public boolean anyhow_to_application_exn;
  public static final int ANYHOW_TO_APPLICATION_EXN = 1;

  // isset id assignments
  private static final int __ANYHOW_TO_APPLICATION_EXN_ISSET_ID = 0;
  private BitSet __isset_bit_vector = new BitSet(1);

  public static final Map<Integer, FieldMetaData> metaDataMap;

  static {
    Map<Integer, FieldMetaData> tmpMetaDataMap = new HashMap<Integer, FieldMetaData>();
    tmpMetaDataMap.put(ANYHOW_TO_APPLICATION_EXN, new FieldMetaData("anyhow_to_application_exn", TFieldRequirementType.DEFAULT, 
        new FieldValueMetaData(TType.BOOL)));
    metaDataMap = Collections.unmodifiableMap(tmpMetaDataMap);
  }

  static {
    FieldMetaData.addStructMetaDataMap(ServiceExn.class, metaDataMap);
  }

  public ServiceExn() {
  }

  public ServiceExn(
      boolean anyhow_to_application_exn) {
    this();
    this.anyhow_to_application_exn = anyhow_to_application_exn;
    setAnyhow_to_application_exnIsSet(true);
  }

  public static class Builder {
    private boolean anyhow_to_application_exn;

    BitSet __optional_isset = new BitSet(1);

    public Builder() {
    }

    public Builder setAnyhow_to_application_exn(final boolean anyhow_to_application_exn) {
      this.anyhow_to_application_exn = anyhow_to_application_exn;
      __optional_isset.set(__ANYHOW_TO_APPLICATION_EXN_ISSET_ID, true);
      return this;
    }

    public ServiceExn build() {
      ServiceExn result = new ServiceExn();
      if (__optional_isset.get(__ANYHOW_TO_APPLICATION_EXN_ISSET_ID)) {
        result.setAnyhow_to_application_exn(this.anyhow_to_application_exn);
      }
      return result;
    }
  }

  public static Builder builder() {
    return new Builder();
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public ServiceExn(ServiceExn other) {
    __isset_bit_vector.clear();
    __isset_bit_vector.or(other.__isset_bit_vector);
    this.anyhow_to_application_exn = TBaseHelper.deepCopy(other.anyhow_to_application_exn);
  }

  public ServiceExn deepCopy() {
    return new ServiceExn(this);
  }

  public boolean isAnyhow_to_application_exn() {
    return this.anyhow_to_application_exn;
  }

  public ServiceExn setAnyhow_to_application_exn(boolean anyhow_to_application_exn) {
    this.anyhow_to_application_exn = anyhow_to_application_exn;
    setAnyhow_to_application_exnIsSet(true);
    return this;
  }

  public void unsetAnyhow_to_application_exn() {
    __isset_bit_vector.clear(__ANYHOW_TO_APPLICATION_EXN_ISSET_ID);
  }

  // Returns true if field anyhow_to_application_exn is set (has been assigned a value) and false otherwise
  public boolean isSetAnyhow_to_application_exn() {
    return __isset_bit_vector.get(__ANYHOW_TO_APPLICATION_EXN_ISSET_ID);
  }

  public void setAnyhow_to_application_exnIsSet(boolean __value) {
    __isset_bit_vector.set(__ANYHOW_TO_APPLICATION_EXN_ISSET_ID, __value);
  }

  public void setFieldValue(int fieldID, Object __value) {
    switch (fieldID) {
    case ANYHOW_TO_APPLICATION_EXN:
      if (__value == null) {
        unsetAnyhow_to_application_exn();
      } else {
        setAnyhow_to_application_exn((Boolean)__value);
      }
      break;

    default:
      throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
    }
  }

  public Object getFieldValue(int fieldID) {
    switch (fieldID) {
    case ANYHOW_TO_APPLICATION_EXN:
      return new Boolean(isAnyhow_to_application_exn());

    default:
      throw new IllegalArgumentException("Field " + fieldID + " doesn't exist!");
    }
  }

  @Override
  public boolean equals(Object _that) {
    if (_that == null)
      return false;
    if (this == _that)
      return true;
    if (!(_that instanceof ServiceExn))
      return false;
    ServiceExn that = (ServiceExn)_that;

    if (!TBaseHelper.equalsNobinary(this.anyhow_to_application_exn, that.anyhow_to_application_exn)) { return false; }

    return true;
  }

  @Override
  public int hashCode() {
    return Arrays.deepHashCode(new Object[] {anyhow_to_application_exn});
  }

  @Override
  public int compareTo(ServiceExn other) {
    if (other == null) {
      // See java.lang.Comparable docs
      throw new NullPointerException();
    }

    if (other == this) {
      return 0;
    }
    int lastComparison = 0;

    lastComparison = Boolean.valueOf(isSetAnyhow_to_application_exn()).compareTo(other.isSetAnyhow_to_application_exn());
    if (lastComparison != 0) {
      return lastComparison;
    }
    lastComparison = TBaseHelper.compareTo(anyhow_to_application_exn, other.anyhow_to_application_exn);
    if (lastComparison != 0) { 
      return lastComparison;
    }
    return 0;
  }

  public void read(TProtocol iprot) throws TException {
    TField __field;
    iprot.readStructBegin(metaDataMap);
    while (true)
    {
      __field = iprot.readFieldBegin();
      if (__field.type == TType.STOP) {
        break;
      }
      switch (__field.id)
      {
        case ANYHOW_TO_APPLICATION_EXN:
          if (__field.type == TType.BOOL) {
            this.anyhow_to_application_exn = iprot.readBool();
            setAnyhow_to_application_exnIsSet(true);
          } else {
            TProtocolUtil.skip(iprot, __field.type);
          }
          break;
        default:
          TProtocolUtil.skip(iprot, __field.type);
          break;
      }
      iprot.readFieldEnd();
    }
    iprot.readStructEnd();


    // check for required fields of primitive type, which can't be checked in the validate method
    validate();
  }

  public void write(TProtocol oprot) throws TException {
    validate();

    oprot.writeStructBegin(STRUCT_DESC);
    oprot.writeFieldBegin(ANYHOW_TO_APPLICATION_EXN_FIELD_DESC);
    oprot.writeBool(this.anyhow_to_application_exn);
    oprot.writeFieldEnd();
    oprot.writeFieldStop();
    oprot.writeStructEnd();
  }

  @Override
  public String toString() {
    return toString(1, true);
  }

  @Override
  public String toString(int indent, boolean prettyPrint) {
    String indentStr = prettyPrint ? TBaseHelper.getIndentedString(indent) : "";
    String newLine = prettyPrint ? "\n" : "";
    String space = prettyPrint ? " " : "";
    StringBuilder sb = new StringBuilder("ServiceExn");
    sb.append(space);
    sb.append("(");
    sb.append(newLine);
    boolean first = true;

    sb.append(indentStr);
    sb.append("anyhow_to_application_exn");
    sb.append(space);
    sb.append(":").append(space);
    sb.append(TBaseHelper.toString(this.isAnyhow_to_application_exn(), indent + 1, prettyPrint));
    first = false;
    sb.append(newLine + TBaseHelper.reduceIndent(indentStr));
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws TException {
    // check for required fields
  }

}

