#!/bin/gawk -f

## Copyright (C) 2015 Red Hat, Inc.
##
## This file is part of elfutils.
##
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## elfutils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

$1 == "#define" {
  if (match($2, "^(ELFOSABI|ET|EM|SHN|SHT|STB|STT" \
	        "|STV|PT|DT|AT|ELF_NOTE_OS|R)_([A-Z0-9_]*)$", fields) \
      && ! match($2, "(_NUM|(LO|HI)(OS|PROC|RESERVE)|RNG(LO|HI))$") \
      && ! match($2, "^(DT_ENCODING|DT_(PROC|VAL|ADDR|VERSIONTAG|EXTRA)NUM)$"))
  {
    set = fields[1];
    elt = fields[2];
    if (set in KNOWN)
      KNOWN[set] = KNOWN[set] "," elt;
    else
      KNOWN[set] = elt;

    if (set == "EM" && elt != "NONE") {
      if (ARCHES) {
	ARCHES = ARCHES "," elt
	ARCHES_RE = ARCHES_RE "|^" elt "_"
      } else {
	ARCHES = elt
	ARCHES_RE = "^" elt "_"
      }
    }
  }
}

function emit(set, elt) {
  print "  ELF_ONE_KNOWN_" set " (" elt ", " set "_" elt ") \\";
}

END {
  print "/* Generated by config/known-elf.awk from libelf/elf.h contents.  */";
  split(ARCHES, arches, ",");
  a = asort(arches)
  n = asorti(KNOWN, sets);
  for (i = 1; i <= n; ++i) {
    set = sets[i];
    split(KNOWN[set], elts, ",");
    m = asort(elts);
    if (m == 0)
      continue;
    print "\n#define ELF_ALL_KNOWN_" set " \\";

    delete known2
    for (j = 1; j <= m; ++j) {
      elt = elts[j];
      found = 0;
      if (set != "ELFOSABI" && set != "EM") {
	# R_ names some architectures in a funny way.  Translate the
	# architecture key back for consistency's sake.  The
	# enumerator with the mangled architecture name is still
	# passed to ELF_ONE_KNOWN_R* invocation.
	if (set "_" elt ~ "^R_390_") {
	  found = 1;
	  arch = "S390";
	} else if (set "_" elt ~ "^R_IA64_") {
	  found = 1;
	  arch = "IA_64";

	} else {
	  for (k = 1; k <= a; ++k) {
	    arch = arches[k];
	    if (elt ~ "^" arches[k] "_") {
	      found = 1;
	      break;
	    }
	  }
	}

	if (found) {
	  if (arch in known2)
	    known2[arch] = known2[arch] "," elt;
	  else
	    known2[arch] = elt;
	}
      }

      if (! found)
	emit(set, elt);
    }

    a2 = asorti(known2, used_arches);
    if (a2 == 0)
      continue;

    print "\n#define ELF_ALL_KNOWN_" set "_ARCHES \\";
    for (k = 1; k <= a2; ++k)
      print "  ELF_ONE_KNOWN_" set "_ARCH (" used_arches[k] ") \\"

    for (k = 1; k <= a2; ++k) {
      arch = used_arches[k];
      set2 = set "_" arch
      split(known2[arch], elts2, ",");
      m2 = asort(elts2);
      if (m2 == 0)
	continue;
      print "\n#define ELF_ALL_KNOWN_" set2 " \\";
      for (j2 = 1; j2 <= m2; ++j2) {
	elt2 = elts2[j2];
	emit(set, elt2);
      }
    }
  }

  print ""
}