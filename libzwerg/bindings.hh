/*
   Copyright (C) 2017 Petr Machata
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

#ifndef _BINDINGS_H_
#define _BINDINGS_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

struct op_bind;

class bindings
{
  std::map <std::string, std::reference_wrapper <op_bind>> m_bindings;
  bindings *m_super;

public:
  bindings ()
    : m_super {nullptr}
  {}

  explicit bindings (bindings &super)
    : m_super {&super}
  {}

  void bind (std::string name, op_bind &op);
  op_bind *find (std::string name);
  std::vector <std::string> names () const;
  std::vector <std::string> names_closure () const;
};

class uprefs
{
  std::map <std::string, std::pair <bool, unsigned>> m_ids;
  unsigned m_nextid;

public:
  uprefs ();
  uprefs (bindings &bns, uprefs &super);

  std::pair <bool, unsigned> find (std::string name);
  std::map <unsigned, std::string> refd_names () const;
};

#endif//_BINDINGS_H_