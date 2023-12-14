/*
  @copyright Steve Keen 2023
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

#ifndef PUBTAB_H
#define PUBTAB_H

#include "item.h"
#include "pannableTab.h"
#include "renderNativeWindow.h"

namespace minsky
{
  class PubItem
  {
  public:
    PubItem()=default;
    PubItem(const ItemPtr& item): itemRef(item) {}
    ItemPtr itemRef;
    float x=100,y=100;
  };

  struct PubTabBase: public RenderNativeWindow
  {
    float offsx=0, offsy=0;
    bool panning=false;
  };
  
  class PubTab: public PannableTab<PubTabBase>
  {
    bool redraw(int x0, int y0, int width, int height) override;
    CLASSDESC_ACCESS(PubTab);
    float m_zoomFactor=1;
    PubItem* item=nullptr; // weak reference for moving items
  public:
    PubTab()=default;
    PubTab(const char* name): name(name) {}
    PubTab(const std::string& name): name(name) {}
    std::string name; ///< name of this publication tab
    std::vector<PubItem> items; ///< list of wrapped items and annotations
    void mouseDown(float x, float y) override;
    void controlMouseDown(float x, float y) override {panning=true; PannableTab<PubTabBase>::mouseDown(x,y);}
    void mouseUp(float x, float y) override;
    void mouseMove(float x, float y) override;
    void zoom(double x, double y, double z) override;
    double zoomFactor() const override;
  };
}

#include "pubTab.cd"
#endif
