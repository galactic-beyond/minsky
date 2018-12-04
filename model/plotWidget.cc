/*
  @copyright Steve Keen 2012
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
#include "plotWidget.h"
#include "variable.h"
#include "cairoItems.h"
#include "minsky.h"
#include "latexMarkup.h"
#include "pango.h"
#include <timer.h>

#include <ecolab_epilogue.h>
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace minsky
{
  namespace
  {
    const unsigned numLines = 4; // number of simultaneous variables to plot, on a side

    const unsigned nBoundsPorts=6;
    // orientation of bounding box ports
    const double orient[nBoundsPorts]={-0.4*M_PI, -0.6*M_PI, -0.2*M_PI, 0.2*M_PI, 1.2*M_PI, 0.8*M_PI};
    // x coordinates of bounding box ports
    const float boundX[nBoundsPorts]={-0.46,0.45,-0.49,-0.49, 0.48, 0.48};
    // y coordinates of bounding box ports
    const float boundY[nBoundsPorts]={0.49,0.49,0.47,-0.49, 0.47, -0.49};

    // height of title, as a fraction of overall widget height
    const double titleHeight=0.07;

    /// temporarily sets nTicks and fontScale, restoring them on scope exit
    struct SetTicksAndFontSize
    {
      PlotWidget& p;
      int nxTicks, nyTicks;
      double fontScale;
      bool subgrid;
      SetTicksAndFontSize(PlotWidget& p, bool override, int n, double f, bool g):
        p(p), nxTicks(p.nxTicks), nyTicks(p.nyTicks), 
        fontScale(p.fontScale), subgrid(p.subgrid) 
      {
        if (override)
          {
            p.nxTicks=p.nyTicks=n;
            p.fontScale=f;
            p.subgrid=g;
          }
      }
      ~SetTicksAndFontSize()
      {
        p.nxTicks=nxTicks;
        p.nyTicks=nyTicks;
        p.fontScale=fontScale;
        p.subgrid=subgrid;
      }
    };

  }

  PlotWidget::PlotWidget()
  {
    // TODO assignPorts();
    nxTicks=nyTicks=10;
    fontScale=2;
    leadingMarker=true;
    grid=true;

    float w=width, h=height;
    float dx=w/(2*numLines+1); // x location of ports
    float dy = h/(numLines);

    // xmin, xmax, ymin, ymax ports
    ports.emplace_back(new Port(*this, Port::inputPort)); //xmin
    ports.emplace_back(new Port(*this, Port::inputPort));  //xmax
    ports.emplace_back(new Port(*this, Port::inputPort)); //ymin
    ports.emplace_back(new Port(*this, Port::inputPort)); //ymax
    ports.emplace_back(new Port(*this, Port::inputPort)); //y1min
    ports.emplace_back(new Port(*this, Port::inputPort)); //y1max

    // y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      ports.emplace_back(new Port(*this, Port::inputPort));

    // RHS y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      ports.emplace_back(new Port(*this, Port::inputPort));

    // add in the x variable ports
    for (float x=2*dx-0.5*w; x<0.5*w; x+=dx)
      ports.emplace_back(new Port(*this, Port::inputPort));

    yvars.resize(2*numLines);
    xvars.resize(numLines);
   }

  void PlotWidget::draw(cairo_t* cairo) const
  {
    double w=width*zoomFactor, h=height*zoomFactor;

    // if any titling, draw an extra bounding box (ticket #285)
    if (!title.empty()||!xlabel.empty()||!ylabel.empty()||!y1label.empty())
      {
        cairo_rectangle(cairo,-0.5*w+10,-0.5*h,w-20,h-10);
        cairo_set_line_width(cairo,1);
        cairo_stroke(cairo);
      }

    cairo_save(cairo);
    cairo_translate(cairo,-0.5*w,-0.5*h);

    double yoffs=0; // offset to allow for title
    if (!title.empty())
      {
        double fx=0, fy=titleHeight*h;
        cairo_user_to_device_distance(cairo,&fx,&fy);
        
        Pango pango(cairo);
        pango.setFontSize(fabs(fy));
        pango.setMarkup(latexToPango(title));   
        cairo_set_source_rgb(cairo,0,0,0);
        cairo_move_to(cairo,0.5*(w-pango.width()), 0/*pango.height()*/);
        pango.show();

        // allow some room for the title
        yoffs=1.2*pango.height();
        h-=1.2*pango.height();
      }

    // draw bounding box ports
    float x = -0.5*w, dx=w/(2*numLines+1); // x location of ports
    float y=0.5*h, dy = h/(numLines);
    
    size_t i=0;
    // draw bounds input ports
    for (; i<nBoundsPorts; ++i)
      {
        float x=boundX[i]*w, y=boundY[i]*h;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i/2)%paletteSz], orient[i]);
        
      }
        
    // draw y data ports
    for (; i<numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-nBoundsPorts)*dy;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], 0);
      }
    
    // draw RHS y data ports
    for (; i<2*numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-numLines-nBoundsPorts)*dy, x=0.5*w;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], M_PI);
      }

    // draw x data ports
    for (; i<4*numLines+nBoundsPorts; ++i)
      {
        float x=dx-0.5*w + (i-2*numLines-nBoundsPorts)*dx;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-2*numLines-nBoundsPorts)%paletteSz], -0.5*M_PI);
      }

    cairo_translate(cairo, 10,yoffs);
    cairo_set_line_width(cairo,1);
    Plot::draw(cairo,w-20,h-10); // allow space for ports
    
    cairo_restore(cairo);
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
      }
    if (onResizeHandles) drawResizeHandles(cairo);
    justDataChanged=false;
    
    cairo_new_path(cairo);
    cairo_rectangle(cairo,-0.5*w,-0.5*h,w,h);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);

  }
  
  void PlotWidget::scalePlot()
  {
    // set any scale overrides
    setMinMax();
    if (xminVar.idx()>-1)
      if (xIsSecsSinceEpoch && xminVar.units==Units("year"))
        minx=yearToPTime(xminVar.value());
      else
        minx=xminVar.value();

    if (xmaxVar.idx()>-1)
      if (xIsSecsSinceEpoch && xmaxVar.units==Units("year"))
        maxx=yearToPTime(xmaxVar.value());
      else
        maxx=xmaxVar.value();

    if (yminVar.idx()>-1) {miny=yminVar.value();}
    if (ymaxVar.idx()>-1) {maxy=ymaxVar.value();}
    if (y1minVar.idx()>-1) {miny1=y1minVar.value();}
    if (y1maxVar.idx()>-1) {maxy1=y1maxVar.value();}
    autoscale=false;

    if (!justDataChanged)
      // label pens
      for (size_t i=0; i<yvars.size(); ++i)
        if (yvars[i].idx()>=0)
          labelPen(i, latexToPango(yvars[i].name));
  }

  extern Tk_Window mainWin;

  void PlotWidget::redraw()
  {
    justDataChanged=true; // assume plot same size, don't do unnecessary stuff
    // store previous min/max values to determine if plot scale changes
    scalePlot();
    if (surface.get())
      surface->requestRedraw();
  }

  void PlotWidget::makeDisplayPlot() {
    if (auto g=group.lock())
      g->displayPlot=dynamic_pointer_cast<PlotWidget>(g->findItem(*this));
  }

  void PlotWidget::resize(const LassoBox& x)
  {
    width=abs(x.x1-x.x0)/zoomFactor;
    height=abs(x.y1-x.y0)/zoomFactor;
    bb.update(*this);
  }

  // specialisation to avoid rerendering plots (potentially expensive)
  ClickType::Type PlotWidget::clickType(float x, float y)
  {
    // firstly, check whether a port has been selected
    for (auto& p: ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*zoomFactor)
          return ClickType::onPort;
      }

    double dx=x-this->x(), dy=y-this->y();
    double w=0.5*width*zoomFactor, h=0.5*height*zoomFactor;
    // check if (x,y) is within portradius of the 4 corners
    if (fabs(fabs(dx)-w) < portRadius*zoomFactor &&
        fabs(fabs(dy)-h) < portRadius*zoomFactor &&
        fabs(hypot(dx,dy)-hypot(w,h)) < portRadius*zoomFactor)
      return ClickType::onResize;
    return (abs(dx)<w && abs(dy)<h)?
      ClickType::onItem: ClickType::outside;
  }
  
  static ptime epoch=microsec_clock::local_time(), accumulatedBlitTime=epoch;

  void PlotWidget::addPlotPt(double t)
  {
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (yvars[pen].numElements()==1 && yvars[pen].idx()>=0)
        {
          double x,y;
          switch (xvars.size())
            {
            case 0: // use t, when x variable not attached
              x=t;
              y=yvars[pen].value();
              break;
            case 1: // use the value of attached variable
              assert(xvars[0].idx()>=0);
              x=xvars[0].value();
              y=yvars[pen].value();
              break;
            default:
              if (pen < xvars.size() && xvars[pen].idx()>=0)
                {
                  x=xvars[pen].value();
                  y=yvars[pen].value();
                }
              else
                throw error("x input not wired for pen %d",(int)pen+1);
              break;
            }
          addPt(pen, x, y);
        }
    
    // throttle plot redraws
    static time_duration maxWait=milliseconds(1000);
    if ((microsec_clock::local_time()-(ptime&)lastAdd) >
        min((accumulatedBlitTime-(ptime&)lastAccumulatedBlitTime) * 2, maxWait))
      {
        ptime timerStart=microsec_clock::local_time();
        redraw();
        lastAccumulatedBlitTime = accumulatedBlitTime;
        lastAdd=microsec_clock::local_time();
        accumulatedBlitTime += lastAdd - timerStart;
      }
  }

  void PlotWidget::addConstantCurves()
  {
    size_t extraPen=2*numLines;

    // determine if any of the incoming vectors has a ptime-based xVector
    xIsSecsSinceEpoch=false;
    for (auto& i: yvars)
      if (i.idx()>=0 && xvars[&i-&yvars[0]].idx()==-1 && i.xVector.size())
        {
          auto& xv=i.xVector[0];
          if (xv.dimension.type==Dimension::time)
            xIsSecsSinceEpoch=true;
        }
    
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (pen<yvars.size() && yvars[pen].numElements()>1 && yvars[pen].idx()>=0)
        {
          auto& yv=yvars[pen];
          auto d=yv.dims();
          if (d.empty()) continue;
          
          // work out a reference to the x data
          vector<double> xdefault;
          double* x;
          if (pen<xvars.size() && xvars[pen].idx()>=0)
            {
              if (xvars[pen].dims()[0]!=d[0])
                throw error("x vector not same length as y vectors");
              x=xvars[pen].begin();
            }
          else
            {
              xdefault.reserve(d[0]);
              xticks.clear();
              if (yv.xVector.size()) // yv carries its own x-vector
                {
                  auto& xv=yv.xVector[0];
                  assert(xv.size()==d[0]);
                  switch (xv.dimension.type)
                    {
                    case Dimension::string:
                      for (size_t i=0; i<xv.size(); ++i)
                        {
                          xticks.emplace_back(i, str(xv[i]));
                          xdefault.push_back(i);
                        }
                      break;
                    case Dimension::value:
                      if (xIsSecsSinceEpoch && xv.dimension.units=="year")
                        // interpret "year" as years since epoch (1/1/1970)
                        for (auto& i: xv)
                          xdefault.push_back(yearToPTime(any_cast<double>(i)));
                      else
                        for (auto& i: xv)
                          xdefault.push_back(any_cast<double>(i));
                      break;
                    case Dimension::time:
                      // choose a sensible format string, dependent on the data
                      string format;
                      static const auto day=hours(24);
                      static const auto month=day*30;
                      static const auto year=day*365;
                      auto dt=any_cast<ptime>(xv.back())-any_cast<ptime>(xv.front());
                      if (dt > year*5)
                        format="%Y";
                      else if (dt > year)
                        format="%b %Y";
                      else if (dt > month*6)
                        format="%b";
                      else if (dt > month)
                        format="%d %b";
                      else if (dt > day)
                        format="%d %H:%M";
                      else if (dt > hours(1))
                        format="%H:%M";
                      else if (dt > minutes(1))
                        format="%M:%S";
                      else
                        format="%s";
                        
                      for (auto& i: xv)
                        {
                          double tv=(any_cast<ptime>(i)-ptime(date(1970,Jan,1))).total_microseconds()*1E-6;
                          xticks.emplace_back(tv,str(i,format));
                          xdefault.push_back(tv);
                        }
                    }
                }
              else // by default, set x to 0..d[0]-1
                for (size_t i=0; i<d[0]; ++i)
                  xdefault.push_back(i);
              x=&xdefault[0];
            }
          
          // higher rank y objects treated as multiple y vectors to plot
          for (size_t j=0 /*d[0]*/; j<std::min(size_t(10)*d[0], yv.numElements()); j+=d[0])
            {
              setPen(extraPen, x, yv.begin()+j, d[0]);
              if (pen>=numLines)
                assignSide(extraPen,Side::right);
             string label;
              size_t stride=d[0];
              for (size_t i=1; i<yv.xVector.size(); ++i)
                {
                  label+=str(yv.xVector[i][(j/stride)%d[i]])+" ";
                  stride*=d[i];
                }
              labelPen(extraPen,label);
              extraPen++;
            }
        }
    scalePlot();
  }

  
  void PlotWidget::connectVar(const VariableValue& var, unsigned port)
  {
    if (port<nBoundsPorts)
      switch (port)
        {
        case 0: xminVar=var; return;
        case 1: xmaxVar=var; return;
        case 2: yminVar=var; return;
        case 3: ymaxVar=var; return;
        case 4: y1minVar=var; return;
        case 5: y1maxVar=var; return;
        }
    unsigned pen=port-nBoundsPorts;
    if (pen<2*numLines)
      {
        yvars.resize(2*numLines);
        yvars[pen]=var;
        if (pen>=numLines)
          assignSide(pen,Side::right);
      }
    else if (pen<4*numLines)
      {
        xvars.resize(2*numLines);
        xvars[pen-2*numLines]=var;
      }
    scalePlot();
  }

  void PlotWidget::disconnectAllVars()
  {
    xvars.clear();
    yvars.clear();
    xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=VariableValue();
  }
}
