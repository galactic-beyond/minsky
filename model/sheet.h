/*
  @copyright Steve Keen 2018
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

/**
   A simple spreadsheet view
 */

#ifndef SHEET_H
#define SHEET_H
#include "item.h"
#include "dynamicRavelCAPI.h"
#include "showSlice.h"

namespace minsky
{
  class Sheet: public ItemT<Sheet>
  {
    
    CLASSDESC_ACCESS(Sheet);
    ravel::Ravel inputRavel; ///< ravel for controlling input
    /// value used for populating sheet. Potentially a tensor expression representing ravel manipulation
    civita::TensorPtr value;
    /// @{ ravel coordinate
    double ravelX(double xx) const {
      return (xx+0.6*m_width-x())*inputRavel.radius()/(0.25*m_width);
    }
    double ravelY(double yy) const {
      return (yy+0.6*m_height-y())*inputRavel.radius()/(0.25*m_height);
    }      
  public:
    Sheet();

    // copy operations needed for clone, but not really used for now
    // define them as empty operations to prevent double frees if accidentally used
    void operator=(const Sheet&){}
    Sheet(const Sheet&) {}
    
    bool onRavelButton(float, float) const;
    bool inRavel(float, float) const;
    bool inItem(float, float) const override;
    void onMouseDown(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseMotion(float x, float y) override;
    bool onMouseOver(float x, float y) override;
    void onMouseLeave() override;
    ClickType::Type clickType(float x, float y) const override;
   
    void draw(cairo_t* cairo) const override;
    
    /// calculates the input value
    void computeValue();
    
    bool showRavel=false;
    ShowSlice showSlice=ShowSlice::head; ///< whether to elide rows from beginning, end or middle
  };
}

#include "sheet.cd"
#include "sheet.xcd"
#endif
