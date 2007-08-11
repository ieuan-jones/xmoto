/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __LOCALES_H__
#define __LOCALES_H__

#include "BuildConfig.h"

#ifndef USE_GETTEXT
#define _(a) a
char* ngettext(char* msgid, char* msgid_plural, unsigned long int n) {
  if(n > 1) {
    return msgid_plural;
  }
  return msgid;
}
#else
#include <locale.h>
#include <libintl.h>
#define _(String) gettext(String)
#endif
#include <string>

class Locales {
  public:
  /* return the locales set */
  static std::string init(std::string i_locale = "");
};

#endif /* __LOCALES_H__ */
