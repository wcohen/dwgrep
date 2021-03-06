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

#ifndef _LAYOUT_HH_
#define _LAYOUT_HH_

#include <cstddef>
#include <vector>

class layout
{
  size_t m_size;

public:
  struct loc
  {
    size_t m_loc;

    loc (size_t loc)
      : m_loc {loc}
    {}
  };

  explicit layout (size_t size = 0)
    : m_size {size}
  {}

  layout::loc reserve (size_t size, size_t align);
  void add_union (std::vector <layout> layouts);
  template <class State> layout::loc reserve ();

  size_t size () const;
};

template <class State>
layout::loc
layout::reserve ()
{
  return reserve (sizeof (State), alignof (State));
}

#endif // _LAYOUT_HH_
