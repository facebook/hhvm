/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

// Functions to handle KEY objects, ie., our internal representation of
// keys as used in SQL indexes.

#include "sql/key.h"  // key_rec_cmp

#include <string.h>
#include <algorithm>

#include "m_ctype.h"
#include "m_string.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compare.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "sql/field.h"  // Field
#include "sql/handler.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/table.h"
#include "sql_string.h"

using std::max;
using std::min;

bool KEY::is_functional_index() const {
  for (uint i = 0; i < actual_key_parts; ++i) {
    if (key_part[i].field->is_field_for_functional_index()) {
      return true;
    }
  }
  return false;
}

/*
  Search after a key that starts with 'field'

  SYNOPSIS
    find_ref_key()
    key			First key to check
    key_count		How many keys to check
    record		Start of record
    field		Field to search after
    key_length		On partial match, contains length of fields before
                        field
    keypart             key part # of a field

  NOTES
   Used when calculating key for NEXT_NUMBER

  IMPLEMENTATION
    If no key starts with field test if field is part of some key. If we find
    one, then return first key and set key_length to the number of bytes
    preceding 'field'.

  RETURN
   -1  field is not part of the key
   #   Key part for key matching key.
       key_length is set to length of key before (not including) field
*/

int find_ref_key(KEY *key, uint key_count, uchar *record, Field *field,
                 uint *key_length, uint *keypart) {
  int i;
  KEY *key_info;
  uint fieldpos;

  fieldpos = field->offset(record);

  /* Test if some key starts as fieldpos */
  for (i = 0, key_info = key; i < (int)key_count; i++, key_info++) {
    if (key_info->key_part[0].offset ==
        fieldpos) { /* Found key. Calc keylength */
      *key_length = *keypart = 0;
      return i; /* Use this key */
    }
  }

  /* Test if some key contains fieldpos */
  for (i = 0, key_info = key; i < (int)key_count; i++, key_info++) {
    uint j;
    KEY_PART_INFO *key_part;
    *key_length = 0;
    for (j = 0, key_part = key_info->key_part;
         j < key_info->user_defined_key_parts; j++, key_part++) {
      if (key_part->offset == fieldpos) {
        *keypart = j;
        return i; /* Use this key */
      }
      *key_length += key_part->store_length;
    }
  }
  return (-1); /* No key is ok */
}

/**
  Copy part of a record that forms a key or key prefix to a buffer.

    The function takes a complete table record (as e.g. retrieved by
    handler::index_read()), and a description of an index on the same table,
    and extracts the first key_length bytes of the record which are part of a
    key into to_key. If length == 0 then copy all bytes from the record that
    form a key.

  @param to_key      buffer that will be used as a key
  @param from_record full record to be copied from
  @param key_info    descriptor of the index
  @param key_length  specifies length of all keyparts that will be copied
*/

void key_copy(uchar *to_key, const uchar *from_record, const KEY *key_info,
              uint key_length) {
  uint length;
  KEY_PART_INFO *key_part;

  if (key_length == 0) key_length = key_info->key_length;
  for (key_part = key_info->key_part; (int)key_length > 0; key_part++) {
    if (key_part->null_bit) {
      bool key_is_null =
          from_record[key_part->null_offset] & key_part->null_bit;
      *to_key++ = (key_is_null ? 1 : 0);
      key_length--;
    }
    if (key_part->key_part_flag & HA_BLOB_PART ||
        key_part->key_part_flag & HA_VAR_LENGTH_PART) {
      key_length -= HA_KEY_BLOB_LENGTH;
      length = min<uint>(key_length, key_part->length);
      key_part->field->get_key_image(to_key, length, Field::itRAW);
      to_key += HA_KEY_BLOB_LENGTH;
    } else {
      length = min<uint>(key_length, key_part->length);
      Field *field = key_part->field;
      const CHARSET_INFO *cs = field->charset();
      size_t bytes = field->get_key_image(to_key, length, Field::itRAW);
      if (bytes < length)
        cs->cset->fill(cs, (char *)to_key + bytes, length - bytes, ' ');
    }
    to_key += length;
    key_length -= length;
  }
}

/**
  Restore a key from some buffer to record.

    This function converts a key into record format. It can be used in cases
    when we want to return a key as a result row.

  @param to_record   record buffer where the key will be restored to
  @param from_key    buffer that contains a key
  @param key_info    descriptor of the index
  @param key_length  specifies length of all keyparts that will be restored
*/

void key_restore(uchar *to_record, const uchar *from_key, const KEY *key_info,
                 uint key_length) {
  uint length;
  KEY_PART_INFO *key_part;

  if (key_length == 0) {
    key_length = key_info->key_length;
  }
  for (key_part = key_info->key_part; (int)key_length > 0; key_part++) {
    uchar used_uneven_bits = 0;
    if (key_part->null_bit) {
      if (*from_key++)
        to_record[key_part->null_offset] |= key_part->null_bit;
      else
        to_record[key_part->null_offset] &= ~key_part->null_bit;
      key_length--;
    }
    if (key_part->type == HA_KEYTYPE_BIT) {
      Field_bit *field = (Field_bit *)(key_part->field);
      if (field->bit_len) {
        uchar bits =
            *(from_key + key_part->length - field->pack_length_in_rec() - 1);
        set_rec_bits(
            bits,
            to_record + key_part->null_offset + (key_part->null_bit == 128),
            field->bit_ofs, field->bit_len);
        /* we have now used the byte with 'uneven' bits */
        used_uneven_bits = 1;
      }
    }
    if (key_part->key_part_flag & HA_BLOB_PART) {
      /*
        This in fact never happens, as we have only partial BLOB
        keys yet anyway, so it's difficult to find any sence to
        restore the part of a record.
        Maybe this branch is to be removed, but now we
        have to ignore GCov compaining.
      */
      uint blob_length = uint2korr(from_key);
      Field_blob *field = (Field_blob *)key_part->field;
      from_key += HA_KEY_BLOB_LENGTH;
      key_length -= HA_KEY_BLOB_LENGTH;
      field->set_ptr_offset(to_record - field->table->record[0],
                            (ulong)blob_length, from_key);
      length = key_part->length;
    } else if (key_part->key_part_flag & HA_VAR_LENGTH_PART) {
      Field *field = key_part->field;
      my_bitmap_map *old_map;
      ptrdiff_t ptrdiff = to_record - field->table->record[0];
      field->move_field_offset(ptrdiff);
      key_length -= HA_KEY_BLOB_LENGTH;
      length = min<uint>(key_length, key_part->length);
      old_map = dbug_tmp_use_all_columns(field->table, field->table->write_set);
      field->set_key_image(from_key, length);
      dbug_tmp_restore_column_map(field->table->write_set, old_map);
      from_key += HA_KEY_BLOB_LENGTH;
      field->move_field_offset(-ptrdiff);
    } else {
      length = min<uint>(key_length, key_part->length);
      /* skip the byte with 'uneven' bits, if used */
      memcpy(to_record + key_part->offset, from_key + used_uneven_bits,
             (size_t)length - used_uneven_bits);
    }
    from_key += length;
    key_length -= length;
  }
}

/**
  Compare if a key has changed.

  @param table		TABLE
  @param key		key to compare to row
  @param idx		Index used
  @param key_length	Length of key

  @note
    In theory we could just call field->cmp() for all field types,
    but as we are only interested if a key has changed (not if the key is
    larger or smaller than the previous value) we can do things a bit
    faster by using memcmp() instead.

  @retval
    0	If key is equal
  @retval
    1	Key has changed
*/

bool key_cmp_if_same(const TABLE *table, const uchar *key, uint idx,
                     uint key_length) {
  uint store_length;
  KEY_PART_INFO *key_part;
  const uchar *key_end = key + key_length;
  ;

  for (key_part = table->key_info[idx].key_part; key < key_end;
       key_part++, key += store_length) {
    store_length = key_part->store_length;

    if (key_part->null_bit) {
      bool key_is_null =
          table->record[0][key_part->null_offset] & key_part->null_bit;
      if (*key != (key_is_null ? 1 : 0)) return true;
      if (*key) continue;
      key++;
      store_length--;
    }
    if (key_part->bin_cmp &&
        !(key_part->key_part_flag &
          (HA_BLOB_PART | HA_VAR_LENGTH_PART | HA_BIT_PART))) {
      // We can use memcpy.
      uint length = min((uint)(key_end - key), store_length);
      if (memcmp(key, table->record[0] + key_part->offset, length)) return true;
    } else {
      // Use the regular comparison function.
      if (key_part->field->key_cmp(key, key_part->length)) return true;
    }
  }
  return false;
}

/**
  Unpack a field and append it.

  @param[in,out] to          String to append the field contents to.
  @param        field        Field to unpack.
  @param        max_length   Maximum length of field to unpack
                             or 0 for unlimited.
  @param        prefix_key   The field is used as a prefix key.
*/

void field_unpack(String *to, Field *field, uint max_length, bool prefix_key) {
  String tmp;
  DBUG_TRACE;
  if (!max_length) max_length = field->pack_length();
  if (field) {
    if (field->is_null()) {
      to->append(STRING_WITH_LEN("NULL"));
      return;
    }
    const CHARSET_INFO *cs = field->charset();
    field->val_str(&tmp);
    /*
      For BINARY(N) strip trailing zeroes to make
      the error message nice-looking
    */
    if (field->binary() && field->type() == MYSQL_TYPE_STRING && tmp.length()) {
      const char *tmp_end = tmp.ptr() + tmp.length();
      while (tmp_end > tmp.ptr() && !*--tmp_end)
        ;
      tmp.length(tmp_end - tmp.ptr() + 1);
    }
    if (cs->mbmaxlen > 1 && prefix_key) {
      /*
        Prefix key, multi-byte charset.
        For the columns of type CHAR(N), the above val_str()
        call will return exactly "key_part->length" bytes,
        which can break a multi-byte characters in the middle.
        Align, returning not more than "char_length" characters.
      */
      size_t charpos, char_length = max_length / cs->mbmaxlen;
      if ((charpos = my_charpos(cs, tmp.ptr(), tmp.ptr() + tmp.length(),
                                char_length)) < tmp.length())
        tmp.length(charpos);
    }
    if (max_length < field->pack_length())
      tmp.length(min(tmp.length(), static_cast<size_t>(max_length)));
    ErrConvString err(&tmp);
    to->append(err.ptr());
  } else
    to->append(STRING_WITH_LEN("???"));
}

/*
  unpack key-fields from record to some buffer.

  This is used mainly to get a good error message.  We temporary
  change the column bitmap so that all columns are readable.

  @param
     to		Store value here in an easy to read form
  @param
     table	Table to use
  @param
     key	Key
*/

void key_unpack(String *to, TABLE *table, KEY *key) {
  my_bitmap_map *old_map = dbug_tmp_use_all_columns(table, table->read_set);
  DBUG_TRACE;

  to->length(0);
  KEY_PART_INFO *key_part_end = key->key_part + key->user_defined_key_parts;
  for (KEY_PART_INFO *key_part = key->key_part; key_part < key_part_end;
       key_part++) {
    if (to->length()) to->append('-');
    if (key_part->null_bit) {
      if (table->record[0][key_part->null_offset] & key_part->null_bit) {
        to->append(STRING_WITH_LEN("NULL"));
        continue;
      }
    }
    field_unpack(to, key_part->field, key_part->length,
                 (key_part->key_part_flag & HA_PART_KEY_SEG));
  }
  dbug_tmp_restore_column_map(table->read_set, old_map);
}

/*
  Check if key uses field that is marked in passed field bitmap.

  SYNOPSIS
    is_key_used()
      table   TABLE object with which keys and fields are associated.
      idx     Key to be checked.
      fields  Bitmap of fields to be checked.

  NOTE
    This function uses TABLE::tmp_set bitmap so the caller should care
    about saving/restoring its state if it also uses this bitmap.

  RETURN VALUE
    true   Key uses field from bitmap
    false  Otherwise
*/

bool is_key_used(TABLE *table, uint idx, const MY_BITMAP *fields) {
  bitmap_clear_all(&table->tmp_set);
  table->mark_columns_used_by_index_no_reset(idx, &table->tmp_set);
  const bool overlapping = bitmap_is_overlapping(&table->tmp_set, fields);

  // Clear tmp_set so it can be used elsewhere
  bitmap_clear_all(&table->tmp_set);
  if (overlapping) return true;

  /*
    If table handler has primary key as part of the index, check that primary
    key is not updated
  */
  if (idx != table->s->primary_key && table->s->primary_key < MAX_KEY &&
      (table->file->ha_table_flags() & HA_PRIMARY_KEY_IN_READ_INDEX))
    return is_key_used(table, table->s->primary_key, fields);
  return false;
}

/**
  Compare key in record buffer to a given key.

  @param key_part		Key part handler
  @param key			Key to compare to value in table->record[0]
  @param key_length		length of 'key'

  @details
    The function compares given key and key in record buffer, part by part,
    using info from key_part arg.
    Since callers expect before/after rather than lesser/greater, result
    depends on the HA_REVERSE_SORT flag of the key part. E.g. For ASC key
    part and two keys, 'A' and 'Z', -1 will be returned. For same keys, but
    DESC key part, 1 will be returned.

  @return
    The return value is SIGN(key_in_row - range_key):
    -   0   Key is equal to record's key
    -  -1   Key is before record's key
    -   1   Key is after record's key

  @note: keep this function and key_cmp2() in sync
*/

int key_cmp(KEY_PART_INFO *key_part, const uchar *key, uint key_length) {
  uint store_length;

  for (const uchar *end = key + key_length; key < end;
       key += store_length, key_part++) {
    int cmp;
    int res = (key_part->key_part_flag & HA_REVERSE_SORT) ? -1 : 1;
    store_length = key_part->store_length;
    if (key_part->null_bit) {
      /* This key part allows null values; NULL is lower than everything */
      bool field_is_null = key_part->field->is_null();
      if (*key)  // If range key is null
      {
        /* the range is expecting a null value */
        if (!field_is_null) return res;  // Found key is > range
        /* null -- exact match, go to next key part */
        continue;
      } else if (field_is_null)
        return -res;  // NULL is less than any value
      key++;          // Skip null byte
      store_length--;
    }
    if ((cmp = key_part->field->key_cmp(key, key_part->length)) < 0)
      return -res;
    if (cmp > 0) return res;
  }
  return 0;  // Keys are equal
}

/**
  Compare two given keys

  @param key_part		Key part handler
  @param key1			Key to be compared with key2
  @param key1_length		length of 'key1'
  @param key2                   Key to be compared with key1
  @param key2_length		length of 'key2'

  @return
    The return value is an integral value that takes into account ASC/DESC
    order of keyparts and indicates the relationship between the two keys:
    -   0                       key1 equal to key2
    -  -1                       Key1 before Key2
    -   1                       Key1 after  Key2
  @note: keep this function and key_cmp() in sync

  Below comparison code is under the assumption
  that key1_length and key2_length are same and
  key1_length, key2_length are non zero value.
  @see key_cmp()
*/
int key_cmp2(KEY_PART_INFO *key_part, const uchar *key1, uint key1_length,
             const uchar *key2, uint key2_length MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(key_part && key1 && key2);
  DBUG_ASSERT((key1_length == key2_length) && key1_length != 0);
  uint store_length;

  /* Compare all the subkeys (if it is a composite key) */
  for (const uchar *end = key1 + key1_length; key1 < end;
       key1 += store_length, key2 += store_length, key_part++) {
    int res = (key_part->key_part_flag & HA_REVERSE_SORT) ? -1 : 1;
    store_length = key_part->store_length;
    /* This key part allows null values; NULL is lower than everything */
    if (key_part->null_bit) {
      if (*key1 != *key2) {
        /*
          Key Format= "1 byte (NULL Indicator flag) + Key value"
          If NULL Indicator flag is '1' that means the key is NULL value
          and If the flag is '0' that means the key is Non-NULL value.

          If null indicating flag in key1 and key2 are not same, then
            > if key1's null flag is '1' (i.e., key1 is NULL), return -1
            > if key1's null flag is '0' (i.e., key1 is NOT NULL), then
              key2's null flag is '1' (since *key1 != *key2) then return 1;
        */
        return (*key1) ? -res : res;
      } else {
        /*
          If null indicating flag in key1 and key2 are same and
            > if it is '1' , both are NULLs and both are same, continue with
              next key in key_part.
            > if it is '0', then go ahead and compare the content using
              field->key_cmp.
        */
        if (*key1) continue;
      }
      /*
        Increment the key1 and key2 pointers to point them to the actual
        key values
      */
      key1++;
      key2++;
      store_length--;
    }
    /* Compare two keys using field->key_cmp */
    int cmp;
    if ((cmp = key_part->field->key_cmp(key1, key2)) < 0) return -res;
    if (cmp > 0) return res;
  }
  return 0; /* Keys are equal */
}

/**
  Compare two records in index order.

  This method is set-up such that it can be called directly from the
  priority queue and it is attempted to be optimised as much as possible
  since this will be called O(N * log N) times while performing a merge
  sort in various places in the code.

  We retrieve the pointer to table->record[0] using the fact that key_parts
  have an offset making it possible to calculate the start of the record.
  We need to get the diff to the compared record since none of the records
  being compared are stored in table->record[0].

  We first check for NULL values, if there are no NULL values we use
  a compare method that gets two field pointers and a max length
  and return the result of the comparison.

  key is a null terminated array, since in some cases (clustered
  primary key) it must compare more than one index.
  We only compare the fields that are specified in table->read_set and
  stop at the first non set field. The first must be set!

  @param key                    Null terminated array of index information
  @param first_rec              Pointer to record compare with
  @param second_rec             Pointer to record compare against first_rec

  @return Return value is less, equal or greater than 0 if first rec is sorted
          before, same or after second rec.
    @retval  0                  Keys are equal
    @retval -1                  second_rec is after first_rec
    @retval +1                  first_rec is after second_rec
*/

int key_rec_cmp(KEY **key, uchar *first_rec, uchar *second_rec) {
  KEY *key_info = *(key++);  // Start with first key
  uint key_parts, key_part_num;
  KEY_PART_INFO *key_part = key_info->key_part;
  uchar *rec0 = key_part->field->ptr - key_part->offset;
  ptrdiff_t first_diff = first_rec - rec0, sec_diff = second_rec - rec0;
  int result = 0;
  Field *field;
  DBUG_TRACE;

  /* Assert that at least the first key part is read. */
  DBUG_ASSERT(bitmap_is_set(key_info->table->read_set,
                            key_info->key_part->field->field_index));
  /* loop over all given keys */
  do {
    key_parts = key_info->user_defined_key_parts;
    key_part = key_info->key_part;
    key_part_num = 0;

    /* loop over every key part */
    do {
      // 1 - ASCENDING order, -1 - DESCENDING
      int sort_order = (key_part->key_part_flag & HA_REVERSE_SORT) ? -1 : 1;

      field = key_part->field;

      /* If not read, compare is done and equal! */
      if (!bitmap_is_set(field->table->read_set, field->field_index)) return 0;

      if (key_part->null_bit) {
        /* The key_part can contain NULL values */
        bool first_is_null = field->is_real_null(first_diff);
        bool sec_is_null = field->is_real_null(sec_diff);
        /*
          NULL is smaller then everything so if first is NULL and the other
          not then we know that we should return -1 and for the opposite
          we should return +1. If both are NULL then we call it equality
          although it is a strange form of equality, we have equally little
          information of the real value.
        */
        if (!first_is_null) {
          if (!sec_is_null)
            ; /* Fall through, no NULL fields */
          else {
            return sort_order;
          }
        } else if (!sec_is_null) {
          return -sort_order;
        } else
          goto next_loop; /* Both were NULL */
      }
      /*
        No null values in the fields
        We use the virtual method cmp_max with a max length parameter.
        For most field types this translates into a cmp without
        max length. The exceptions are the BLOB and VARCHAR field types
        that take the max length into account.
      */
      if ((result = field->cmp_max(field->ptr + first_diff,
                                   field->ptr + sec_diff, key_part->length)))
        return (sort_order < 0) ? -result : result;
    next_loop:
      key_part++;
      key_part_num++;
    } while (key_part_num < key_parts); /* this key is done */

    key_info = *(key++);
  } while (key_info); /* no more keys to test */
  return 0;
}
