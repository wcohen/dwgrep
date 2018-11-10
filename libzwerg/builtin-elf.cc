/*
   Copyright (C) 2018 Petr Machata
   This file is part of dwgrep.

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

#include "builtin-elf.hh"
#include "dwpp.hh"
#include "dwcst.hh"
#include "elfcst.hh"

namespace
{
  int
  find_no_debuginfo (Dwfl_Module *mod, void **userdata,
		     const char *modname, Dwarf_Addr base,
		     const char *file_name,
		     const char *debuglink_file, GElf_Word debuglink_crc,
		     char **debuginfo_file_name)
  {
    *debuginfo_file_name = NULL;
    return -ENOENT;
  }

  struct elf_producer
    : public value_producer <value_elf>
  {
    std::unique_ptr <value_dwarf> m_dw;
    char const *m_mainfile;
    char const *m_debugfile;
    unsigned m_pos;

    explicit elf_producer (std::unique_ptr <value_dwarf> dw)
      : m_dw {std::move (dw)}
      , m_pos {0}
    {
      Dwfl_Module *mod = get_sole_module (m_dw->get_dwctx ()->get_dwfl ());
      dwfl_module_info (mod, NULL, NULL, NULL, NULL, NULL,
			&m_mainfile, &m_debugfile);
    }

    std::unique_ptr <value_elf>
    yield_elf (char const **namep)
    {
      std::string fn = *namep;
      *namep = nullptr;

      const static Dwfl_Callbacks callbacks =
	{
	  .find_elf = dwfl_build_id_find_elf,
	  .find_debuginfo = find_no_debuginfo,
	  .section_address = dwfl_offline_section_address,
	};

      auto ctx = std::make_shared <dwfl_context> (open_dwfl (fn, callbacks));
      return std::make_unique <value_elf> (ctx, fn, m_pos++,
					   m_dw->get_doneness ());
    }

    std::unique_ptr <value_elf>
    next () override
    {
      if (m_mainfile)
	return yield_elf (&m_mainfile);
      else if (m_debugfile)
	return yield_elf (&m_debugfile);
      else
	return nullptr;
    }
  };
}

std::unique_ptr <value_producer <value_elf>>
op_elf_dwarf::operate (std::unique_ptr <value_dwarf> val) const
{
  return std::make_unique <elf_producer> (std::move (val));
}

std::string
op_elf_dwarf::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


value_str
op_name_elf::operate (std::unique_ptr <value_elf> a) const
{
  return value_str {std::string {a->get_fn ()}, 0};
}

std::string
op_name_elf::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}

namespace
{
  GElf_Ehdr
  ehdr (Dwfl *dwfl)
  {
    Elf *elf = get_main_elf (dwfl).first;
    // Note: Bias ignored.
    GElf_Ehdr ehdr;
    if (gelf_getehdr (elf, &ehdr) == nullptr)
      throw_libelf ();

    return ehdr;
  }
}

unsigned
elf_eclass_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_ident[EI_CLASS];
}

zw_cdom const &
elf_eclass_def::cdom ()
{
  return elf_class_dom ();
}

std::string
elf_eclass_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


unsigned
elf_edata_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_ident[EI_DATA];
}

zw_cdom const &
elf_edata_def::cdom ()
{
  return elf_data_dom ();
}

std::string
elf_edata_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


unsigned
elf_etype_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_type;
}

zw_cdom const &
elf_etype_def::cdom ()
{
  return elf_et_dom ();
}

std::string
elf_etype_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


unsigned
elf_emachine_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_machine;
}

zw_cdom const &
elf_emachine_def::cdom ()
{
  return elf_em_dom ();
}

std::string
elf_emachine_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


unsigned
elf_eflags_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_flags;
}

zw_cdom const &
elf_eflags_def::cdom ()
{
  // Since it's not clear how the bit set in e_flags breaks down to individual
  // flags (there are various arch-specific bit subfields in there), present the
  // flags simply as a hex number.
  return hex_constant_dom;
}

std::string
elf_eflags_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


unsigned
elf_osabi_def::value (Dwfl *dwfl)
{
  return ehdr (dwfl).e_ident[EI_OSABI];
}

zw_cdom const &
elf_osabi_def::cdom ()
{
  return elf_osabi_dom ();
}

std::string
elf_osabi_def::docstring ()
{
  return
R"docstring(

xxx document me

)docstring";
}


template <class ValueType>
value_cst
op_version_elf <ValueType>::operate (std::unique_ptr <ValueType> a) const
{
  auto ee = ehdr (a->get_dwctx ()->get_dwfl ()).e_ident[EI_VERSION];
  return value_cst {constant {ee, &elf_ev_dom ()}, 0};
}

template <class ValueType>
std::string
op_version_elf <ValueType>::docstring ()
{
  return
R"docstring(

This word takes the ``T_DWARF`` or ``T_ELF`` value on TOS and yields an ELF
header version number from the ELF identifier.

)docstring";
}

template class op_version_elf <value_elf>;
template class op_version_elf <value_dwarf>;


template <class ValueType>
value_cst
op_eversion_elf <ValueType>::operate (std::unique_ptr <ValueType> a) const
{
  auto ee = ehdr (a->get_dwctx ()->get_dwfl ()).e_version;
  return value_cst {constant {ee, &elf_ev_dom ()}, 0};
}

template <class ValueType>
std::string
op_eversion_elf <ValueType>::docstring ()
{
  return
R"docstring(

This word takes the ``T_DWARF`` or ``T_ELF`` value on TOS and yields an object
file version. This is unlike ``version``, which yields ELF header version.

)docstring";
}

template class op_eversion_elf <value_elf>;
template class op_eversion_elf <value_dwarf>;


template <class ValueType>
value_cst
op_eentry_elf <ValueType>::operate (std::unique_ptr <ValueType> a) const
{
  auto ee = ehdr (a->get_dwctx ()->get_dwfl ()).e_entry;
  return value_cst {constant {ee, &dw_address_dom ()}, 0};
}

template <class ValueType>
std::string
op_eentry_elf <ValueType>::docstring ()
{
  return
R"docstring(

This word takes the ``T_DWARF`` or ``T_ELF`` value on TOS and yields an ELF
entry point address.

)docstring";
}

template class op_eentry_elf <value_elf>;
template class op_eentry_elf <value_dwarf>;


template <class ValueType>
value_cst
op_abiversion_elf <ValueType>::operate (std::unique_ptr <ValueType> a) const
{
  auto ee = ehdr (a->get_dwctx ()->get_dwfl ()).e_ident[EI_ABIVERSION];
  return value_cst {constant {ee, &dec_constant_dom}, 0};
}

template <class ValueType>
std::string
op_abiversion_elf <ValueType>::docstring ()
{
  return
R"docstring(

This word takes the ``T_DWARF`` or ``T_ELF`` value on TOS and yields an ABI
version, as stored on index ``EI_ABIVERSION`` of ELF identification bytes.

)docstring";
}

template class op_abiversion_elf <value_elf>;
template class op_abiversion_elf <value_dwarf>;


template <class ValueType>
value_seq
op_eident_elf <ValueType>::operate (std::unique_ptr <ValueType> a) const
{
  auto const &eh = ehdr (a->get_dwctx ()->get_dwfl ());
  std::vector <std::unique_ptr <value>> seq;

  for (unsigned i = 0; i < EI_NIDENT; ++i)
    {
      auto e = eh.e_ident[i];
      auto c = constant {e, &dec_constant_dom};
      auto v = std::make_unique <value_cst> (c, i);
      seq.push_back (std::move (v));
    }

  return {std::move (seq), 0};
}

template <class ValueType>
std::string
op_eident_elf <ValueType>::docstring ()
{
  return
R"docstring(

This word takes the ``T_DWARF`` or ``T_ELF`` value on TOS and yields an ELF
identification array. A suite of constants ``EI_*`` can be used to match
individual elements in the identification sequence::

	$ dwgrep foo.o -e 'eident elem (pos == EI_VERSION)'
	1

Note that you can access some fields of the ELF identifier using the words
``eclass``, ``edata``, ``version``, ``osabi`` and ``abiversion``, instead of
inspecting the identification array explicitly.

)docstring";
}

template class op_eident_elf <value_elf>;
template class op_eident_elf <value_dwarf>;
