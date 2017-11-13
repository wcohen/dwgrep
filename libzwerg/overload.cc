/*
   Copyright (C) 2017 Petr Machata
   Copyright (C) 2014 Red Hat, Inc.
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

#include <memory>
#include <algorithm>
#include <iterator>

#include "overload.hh"
#include "docstring.hh"

overload_instance::overload_instance
	(std::vector <std::tuple <selector,
				  std::shared_ptr <builtin>>> const &stencil)
{
  for (auto const &v: stencil)
    {
      auto origin = std::make_shared <op_origin> ();
      auto op = std::get <1> (v)->build_exec (origin);
      auto pred = std::get <1> (v)->build_pred ();

      assert (op != nullptr || pred != nullptr);

      if (op == nullptr)
	origin = nullptr;

      m_selectors.push_back (std::get <0> (v));
      m_execs.push_back (std::make_tuple (origin, op, op->reserve ()));
      m_preds.push_back (std::move (pred));
    }
}

size_t
overload_instance::max_reserve () const
{
  size_t max = 0;
  for (auto const &exec: m_execs)
    if (std::get <2> (exec) > max)
      max = std::get <2> (exec);
  return max;
}

namespace
{
  ssize_t
  find_selector (selector profile, std::vector <selector> const &selectors)
  {
    auto it = std::find_if (selectors.begin (), selectors.end (),
			    [profile] (selector const &sel)
			    { return sel.matches (profile); });
    if (it == selectors.end ())
      return -1;
    else
      return it - selectors.begin ();
  }
}

std::tuple <op_origin *, op *, size_t>
overload_instance::find_exec (stack &stk) const
{
  ssize_t idx = find_selector (selector {stk}, m_selectors);
  if (idx < 0)
    return {nullptr, nullptr, 0};
  else
    return {std::get <0> (m_execs[idx]).get (),
	    std::get <1> (m_execs[idx]).get (),
	    std::get <2> (m_execs[idx])};
}

std::shared_ptr <pred>
overload_instance::find_pred (stack &stk) const
{
  ssize_t idx = find_selector (selector {stk}, m_selectors);
  if (idx < 0)
    return nullptr;
  else
    return m_preds[idx];
}

static void
show_expects (std::string const &name, std::vector <selector> selectors,
	      selector profile)
{
  std::cerr << "Error: `" << name << "'";

  if (selectors.empty ())
    {
      std::cerr << " has no registered overloads.\n";
      return;
    }

  std::cerr << " expects ";
  for (size_t i = 0; i < selectors.size (); ++i)
    {
      if (i == 0)
	;
      else if (i == selectors.size () - 1)
	std::cerr << " or ";
      else
	std::cerr << ", ";

      std::cerr << selectors[i];
    }
  std::cerr << " near TOS.  Actual profile is " << profile << ".\n";
}

void
overload_instance::show_error (std::string const &name, selector profile) const
{
  return show_expects (name, m_selectors, profile);
}

overload_tab::overload_tab (overload_tab const &a, overload_tab const &b)
  : overload_tab {a}
{
  for (auto const &overload: b.m_overloads)
    add_overload (std::get <0> (overload), std::get <1> (overload));
}

void
overload_tab::add_overload (selector sel, std::shared_ptr <builtin> b)
{
  // Check someone didn't order overload for this type yet.
  for (auto const &ovl: m_overloads)
    assert (std::get <0> (ovl) != sel);

  m_overloads.push_back (std::make_tuple (sel, b));
}

overload_instance
overload_tab::instantiate ()
{
  return overload_instance {m_overloads};
}


struct overload_op::state
{
  op *m_op;

  state ()
    : m_op {nullptr}
  {}
};

overload_op::overload_op (std::shared_ptr <op> upstream,
			  overload_instance ovl_inst)
  : m_upstream {upstream}
  , m_ovl_inst {ovl_inst}
{}

size_t
overload_op::reserve () const
{
  return m_upstream->reserve () + m_ovl_inst.max_reserve () + sizeof (state);
}

void
overload_op::state_con (void *buf) const
{
  state *st = this_state <state> (buf);
  new (st) state {};
  // next() manages state for the invoked overload.
  auto ptr = reinterpret_cast <uint8_t *> (op::next_state (st));
  ptr += m_ovl_inst.max_reserve ();
  m_upstream->state_con (ptr);
}

void
overload_op::state_des (void *buf) const
{
  state *st = this_state <state> (buf);
  // next() manages state for the invoked overload.
  auto ptr = reinterpret_cast <uint8_t *> (op::next_state (st));
  ptr += m_ovl_inst.max_reserve ();
  m_upstream->state_des (ptr);
  st->~state ();
}

stack::uptr
overload_op::next (void *buf) const
{
  state *st = this_state <state> (buf);
  auto nst = reinterpret_cast <uint8_t *> (op::next_state (st));
  auto ust = nst + m_ovl_inst.max_reserve ();
  while (true)
    {
      while (st->m_op == nullptr)
	{
	  if (auto stk = m_upstream->next (ust))
	    {
	      auto ovl = m_ovl_inst.find_exec (*stk);
	      if (std::get <0> (ovl) == nullptr)
		m_ovl_inst.show_error (name (), selector {*stk});
	      else
		{
		  st->m_op = std::get <1> (ovl);
		  st->m_op->state_con (nst);
		  void *end_nst = nst + std::get <2> (ovl);
		  std::get <0> (ovl)->set_next (end_nst, std::move (stk));
		}
	    }
	  else
	    return nullptr;
	}

      if (auto stk = st->m_op->next (nst))
	return stk;

      st->m_op->state_des (op::next_state (st));
      st->m_op = nullptr;
    }
}


pred_result
overload_pred::result (stack &stk)
{
  auto ovl = m_ovl_inst.find_pred (stk);
  if (ovl == nullptr)
    {
      m_ovl_inst.show_error (name (), selector {stk});
      return pred_result::fail;
    }
  else
    return ovl->result (stk);
}

namespace
{
  struct named_overload_op
    : public overload_op
  {
    char const *m_name;

    named_overload_op (std::shared_ptr <op> upstream,
		       overload_instance ovl_inst,
		       char const *name)
      : overload_op {upstream, ovl_inst}
      , m_name {name}
    {}

    std::string
    name () const override
    {
      return m_name;
    }
  };
}

std::string
overloaded_builtin::docstring () const
{
  std::vector <std::pair <std::string, std::string>> entries;

  auto const &ovls = m_ovl_tab->get_overloads ();
  std::transform (ovls.begin (), ovls.end (), std::back_inserter (entries),
		  format_overload);

  // If all overloads are @hide, we want to hide the whole operator,
  // not render an empty one.
  if (std::all_of (entries.begin (), entries.end (),
		   [] (std::pair <std::string, std::string> const &e)
		   { return e.second == "@hide"; }))
    return "@hide";

  return format_entry_map (doc_deduplicate (entries), '.');
}

std::shared_ptr <op>
overloaded_op_builtin::build_exec (std::shared_ptr <op> upstream) const
{
  return std::make_shared <named_overload_op>
    (upstream, get_overload_tab ()->instantiate (), name ());
}

std::shared_ptr <overloaded_builtin>
overloaded_op_builtin::create_merged (std::shared_ptr <overload_tab> tab) const
{
  return std::make_shared <overloaded_op_builtin> (name (), tab);
}

namespace
{
  struct named_overload_pred
    : public overload_pred
  {
    char const *m_name;

    named_overload_pred (overload_instance ovl_inst, char const *name)
      : overload_pred {ovl_inst}
      , m_name {name}
    {}

    std::string
    name () const override
    {
      return m_name;
    }
  };
}

std::unique_ptr <pred>
overloaded_pred_builtin::build_pred () const
{
  return maybe_invert (std::make_unique <named_overload_pred>
				(get_overload_tab ()->instantiate (), name ()),
		       m_positive);
}

std::shared_ptr <overloaded_builtin>
overloaded_pred_builtin::create_merged
	(std::shared_ptr <overload_tab> tab) const
{
  return std::make_shared <overloaded_pred_builtin> (name (), tab, m_positive);
}
