/*
  Copyright (C) 2014, 2015 Red Hat, Inc.

  This file is free software; you can redistribute it and/or modify
  it under the terms of either

  * the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 3 of the License, or (at
  your option) any later version

  or

  * the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at
  your option) any later version

  or both in parallel, as here.

  dwgrep is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received copies of the GNU General Public License and
  the GNU Lesser General Public License along with this program.  If
  not, see <http://www.gnu.org/licenses/>.  */

#ifndef LIBZWERG_DW_H_
#define LIBZWERG_DW_H_

#include <libzwerg.h>
#include <elfutils/libdwfl.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct zw_machine zw_machine;

  zw_machine *zw_machine_init (int code);

  void zw_machine_destroy (zw_machine *machine, zw_error **out_err);

  int zw_machine_code (zw_machine const *machine);

  /**
   * Constant domains.
   */

  zw_cdom const *zw_cdom_dw_tag (void);
  zw_cdom const *zw_cdom_dw_attr (void);
  zw_cdom const *zw_cdom_dw_form (void);
  zw_cdom const *zw_cdom_dw_lang (void);
  zw_cdom const *zw_cdom_dw_macinfo (void);
  zw_cdom const *zw_cdom_dw_macro (void);
  zw_cdom const *zw_cdom_dw_inline (void);
  zw_cdom const *zw_cdom_dw_encoding (void);
  zw_cdom const *zw_cdom_dw_access (void);
  zw_cdom const *zw_cdom_dw_visibility (void);
  zw_cdom const *zw_cdom_dw_virtuality (void);
  zw_cdom const *zw_cdom_dw_identifier_case (void);
  zw_cdom const *zw_cdom_dw_calling_convention (void);
  zw_cdom const *zw_cdom_dw_ordering (void);
  zw_cdom const *zw_cdom_dw_discr_list (void);
  zw_cdom const *zw_cdom_dw_decimal_sign (void);
  zw_cdom const *zw_cdom_dw_locexpr_opcode (void);
  zw_cdom const *zw_cdom_dw_address_class (void);
  zw_cdom const *zw_cdom_dw_endianity (void);

  zw_cdom const *zw_cdom_elfsym_stt (zw_machine const *machine);
  zw_cdom const *zw_cdom_elfsym_stb (zw_machine const *machine);
  zw_cdom const *zw_cdom_elfsym_stv (void);

  /**
   * Dwarf.
   */

  zw_value *zw_value_init_dwarf (char const *filename,
				 size_t pos, zw_error **out_err);

  zw_value *zw_value_init_dwarf_raw (char const *filename,
				     size_t pos, zw_error **out_err);

  zw_value *zw_value_init_dwarf_dwfl (Dwfl *dwfl, char const *name,
				      size_t pos, zw_error **out_err);

  zw_value *zw_value_init_dwarf_dwfl_raw (Dwfl *dwfl, char const *name,
					  size_t pos, zw_error **out_err);

  bool zw_value_is_dwarf (zw_value const *val);

  Dwfl *zw_value_dwarf_dwfl (zw_value const *dw);

  char const *zw_value_dwarf_name (zw_value const *dw);

  zw_machine const *zw_value_dwarf_machine (zw_value const *dw,
					    zw_error **out_err);


  /**
   * CU.
   */

  bool zw_value_is_cu (zw_value const *val);

  Dwarf_CU *zw_value_cu_cu (zw_value const *val);

  Dwarf_Off zw_value_cu_offset (zw_value const *val);


  /**
   * DIE.
   */

  zw_value *zw_value_init_die (zw_value const *dw, Dwarf_Die die,
			       zw_value const *import_die,
			       size_t pos, zw_error **out_err);

  zw_value *zw_value_init_die_raw (zw_value const *dw, Dwarf_Die die,
				   size_t pos, zw_error **out_err);

  bool zw_value_is_die (zw_value const *val);

  Dwarf_Die zw_value_die_die (zw_value const *die);

  zw_value const *zw_value_die_dwarf (zw_value const *die, zw_error **out_err);


  /**
   * DIE attribute.
   */

  bool zw_value_is_attr (zw_value const *val);

  Dwarf_Attribute zw_value_attr_attr (zw_value const *attr);

  zw_value const *zw_value_attr_dwarf (zw_value const *attr,
				       zw_error **out_err);


  /**
   * Location list element.
   */

  bool zw_value_is_llelem (zw_value const *val);

  Dwarf_Attribute zw_value_llelem_attribute (zw_value const *llelem);

  Dwarf_Addr zw_value_llelem_low (zw_value const *llelem);

  Dwarf_Addr zw_value_llelem_high (zw_value const *llelem);

  Dwarf_Op *zw_value_llelem_expr (zw_value const *llelem, size_t *out_length);


  /**
   * Location list operation.
   */

  bool zw_value_is_llop (zw_value const *val);

  Dwarf_Attribute zw_value_llop_attribute (zw_value const *llop);

  Dwarf_Op *zw_value_llop_op (zw_value const *llop);


  /**
   * Address sets.
   */

  bool zw_value_is_aset (zw_value const *val);

  size_t zw_value_aset_length (zw_value const *aset);

  struct zw_aset_pair
  {
    Dwarf_Addr start;
    Dwarf_Word length;
  };

  struct zw_aset_pair zw_value_aset_at (zw_value const *aset, size_t idx);


  /**
   * ELF symbols.
   */

  bool zw_value_is_elfsym (zw_value const *val);

  unsigned zw_value_elfsym_symidx (zw_value const *elfsym);

  char const *zw_value_elfsym_name (zw_value const *elfsym);

  GElf_Sym zw_value_elfsym_symbol (zw_value const *elfsym);

  Elf *zw_value_elfsym_elf (zw_value const *elfsym);


#ifdef __cplusplus
}
#endif

#endif
