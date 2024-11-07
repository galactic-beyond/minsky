/*
  @copyright Steve Keen 2015
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NOTEBASE_H
#define NOTEBASE_H

#include "classdesc_access.h"
#include <string>

namespace minsky 
{
  class NoteBase
  {
  public:
    mutable bool mouseFocus=false; ///<true if target of a mouseover
    bool selected=false; ///<true if selected for cut, copy or group operation
    bool bookmark=false; ///< Is this item also a bookmark?
    virtual std::string const& detailedText() const {return m_detailedText;}
    virtual std::string const& detailedText(const std::string& x) {return m_detailedText=x;}
    virtual std::string const& tooltip() const {return m_tooltip;}
    virtual std::string const& tooltip(const std::string& x) {return m_tooltip=x;}
    /// adjust bookmark list to reflect current configuration
    virtual void adjustBookmark() const {}
    virtual void updateBoundingBox() {}
    virtual ~NoteBase()=default;
  private:
    CLASSDESC_ACCESS(NoteBase);
    std::string m_detailedText, m_tooltip;
  };
}

#include "noteBase.cd"
#endif
