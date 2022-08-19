#ifndef ITEM_INETFUNC_INCLUDED
#define ITEM_INETFUNC_INCLUDED

/* Copyright (c) 2011, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "m_ctype.h"
#include "my_inttypes.h"
#include "sql/item_cmpfunc.h"  // Item_bool_func
#include "sql/item_func.h"
#include "sql/item_strfunc.h"  // Item_str_func
#include "sql/parse_tree_node_base.h"

class Item;
class String;
class THD;

/*************************************************************************
  Item_func_inet_aton implements INET_ATON() SQL-function.
*************************************************************************/

class Item_func_inet_aton : public Item_int_func {
 public:
  inline Item_func_inet_aton(const POS &pos, Item *arg)
      : Item_int_func(pos, arg) {}

 public:
  longlong val_int() override;

  const char *func_name() const override { return "inet_aton"; }

  bool resolve_type(THD *) override {
    maybe_null = true;
    unsigned_flag = true;
    return false;
  }
};

/*************************************************************************
  Item_func_inet_ntoa implements INET_NTOA() SQL-function.
*************************************************************************/

class Item_func_inet_ntoa : public Item_str_func {
 public:
  inline Item_func_inet_ntoa(const POS &pos, Item *arg)
      : Item_str_func(pos, arg) {}

 public:
  String *val_str(String *str) override;

  const char *func_name() const override { return "inet_ntoa"; }

  bool resolve_type(THD *) override {
    set_data_type_string(3 * 8 + 7, default_charset());
    maybe_null = true;
    return false;
  }
};

/*************************************************************************
  Item_func_inet_bool_base implements common code for INET6/IP-related
  functions returning boolean value.
*************************************************************************/

class Item_func_inet_bool_base : public Item_bool_func {
 public:
  Item_func_inet_bool_base(const POS &pos, Item *ip_addr)
      : Item_bool_func(pos, ip_addr) {
    null_value = false;
  }

 public:
  longlong val_int() override;

 protected:
  virtual bool calc_value(const String *arg) const = 0;
};

/*************************************************************************
  Item_func_inet_str_base implements common code for INET6/IP-related
  functions returning string value.
*************************************************************************/

class Item_func_inet_str_base : public Item_str_ascii_func {
 public:
  Item_func_inet_str_base(const POS &pos, Item *arg)
      : Item_str_ascii_func(pos, arg) {}

 public:
  String *val_str_ascii(String *buffer) override;

 protected:
  virtual bool calc_value(String *arg, String *buffer) = 0;
};

/*************************************************************************
  Item_func_inet6_aton implements INET6_ATON() SQL-function.
*************************************************************************/

class Item_func_inet6_aton : public Item_func_inet_str_base {
 public:
  Item_func_inet6_aton(const POS &pos, Item *ip_addr)
      : Item_func_inet_str_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "inet6_aton"; }

  bool resolve_type(THD *) override {
    set_data_type_string(16, &my_charset_bin);
    maybe_null = true;
    return false;
  }

 protected:
  bool calc_value(String *arg, String *buffer) override;
};

/*************************************************************************
  Item_func_inet6_ntoa implements INET6_NTOA() SQL-function.
*************************************************************************/

class Item_func_inet6_ntoa : public Item_func_inet_str_base {
 public:
  Item_func_inet6_ntoa(const POS &pos, Item *ip_addr)
      : Item_func_inet_str_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "inet6_ntoa"; }

  bool resolve_type(THD *) override {
    // max length: IPv6-address -- 16 bytes
    // 16 bytes / 2 bytes per group == 8 groups => 7 delimiter
    // 4 symbols per group
    set_data_type_string(8 * 4 + 7, default_charset());

    maybe_null = true;
    return false;
  }

 protected:
  bool calc_value(String *arg, String *buffer) override;
};

/*************************************************************************
  Item_func_is_ipv4 implements IS_IPV4() SQL-function.
*************************************************************************/

class Item_func_is_ipv4 : public Item_func_inet_bool_base {
 public:
  Item_func_is_ipv4(const POS &pos, Item *ip_addr)
      : Item_func_inet_bool_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "is_ipv4"; }

 protected:
  bool calc_value(const String *arg) const override;
};

/*************************************************************************
  Item_func_is_ipv6 implements IS_IPV6() SQL-function.
*************************************************************************/

class Item_func_is_ipv6 : public Item_func_inet_bool_base {
 public:
  Item_func_is_ipv6(const POS &pos, Item *ip_addr)
      : Item_func_inet_bool_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "is_ipv6"; }

 protected:
  bool calc_value(const String *arg) const override;
};

/*************************************************************************
  Item_func_is_ipv4_compat implements IS_IPV4_COMPAT() SQL-function.
*************************************************************************/

class Item_func_is_ipv4_compat : public Item_func_inet_bool_base {
 public:
  Item_func_is_ipv4_compat(const POS &pos, Item *ip_addr)
      : Item_func_inet_bool_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "is_ipv4_compat"; }

 protected:
  bool calc_value(const String *arg) const override;
};

/*************************************************************************
  Item_func_is_ipv4_mapped implements IS_IPV4_MAPPED() SQL-function.
*************************************************************************/

class Item_func_is_ipv4_mapped : public Item_func_inet_bool_base {
 public:
  Item_func_is_ipv4_mapped(const POS &pos, Item *ip_addr)
      : Item_func_inet_bool_base(pos, ip_addr) {}

 public:
  const char *func_name() const override { return "is_ipv4_mapped"; }

 protected:
  bool calc_value(const String *arg) const override;
};

#endif  // ITEM_INETFUNC_INCLUDED
