
#ifndef SOURCE_CANVAS_HXX
#define SOURCE_CANVAS_HXX

#include <gtkmm/drawingarea.h>

#include <vector>
#include <sstream>
#include <iostream>

#include <gdkmm.h>

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

using namespace std;

class Canvas : public Gtk::DrawingArea
{
  public:
    // core GUI instance writes to these on init
    LV2UI_Controller controller;
    LV2UI_Write_Function write_function;
    
    Canvas()
    {
      width = 956;
      height = 546;
      set_size_request( width, height );
      
      loadHeaderImage();
      
      // connect GTK signals
      add_events( Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK| Gdk::POINTER_MOTION_MASK );
      
      signal_motion_notify_event() .connect( sigc::mem_fun(*this, &Canvas::on_mouse_move ) );
      signal_button_press_event()  .connect( sigc::mem_fun(*this, &Canvas::on_button_press_event) );
      signal_button_release_event().connect( sigc::mem_fun(*this, &Canvas::on_button_release_event) );
    }
    
    void drawMaster(Cairo::RefPtr<Cairo::Context> cr);
    void drawRemove(Cairo::RefPtr<Cairo::Context> cr);
    
    bool redraw()
    {
      Glib::RefPtr<Gdk::Window> win = get_window();
      if (win)
      {
          Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                  get_allocation().get_height());
          win->invalidate_rect(r, false);
      }
      return true;
    }
    
    // will redraw a portion of the screen
    bool redraw(int x, int y, int sx, int sy)
    {
      Glib::RefPtr<Gdk::Window> win = get_window();
      if (win)
      {
          Gdk::Rectangle r(x,y,sx,sy);
          win->invalidate_rect(r, false);
      }
      return true;
    }
    
  protected:
    
    int width, height;
    
    // Image header
    bool headerLoaded;
    Glib::RefPtr< Gdk::Pixbuf > imagePointer;
    Cairo::RefPtr< Cairo::ImageSurface > imageSurfacePointer;
    
    enum Colour {
      COLOUR_ORANGE_1 = 0,
      COLOUR_ORANGE_2,
      COLOUR_ORANGE_3,
      COLOUR_GREEN_1,
      COLOUR_GREEN_2,
      COLOUR_GREEN_3,
      COLOUR_BLUE_1,
      COLOUR_BLUE_2,
      COLOUR_BLUE_3,
      COLOUR_PURPLE_1,
      COLOUR_PURPLE_2,
      COLOUR_PURPLE_3,
      COLOUR_GREY_1,
      COLOUR_GREY_2,
      COLOUR_GREY_3,
      COLOUR_GREY_4,
      // specials
      COLOUR_BACKGROUND,
      COLOUR_RECORD_RED,
      COLOUR_TRANSPARENT,
    };
    
    void loadHeaderImage()
    {
      // Load pixbuf
      try {
        imagePointer = Gdk::Pixbuf::create_from_file ("/usr/lib/lv2/source.lv2/header.png");
        headerLoaded = true;
      }
      catch(Glib::FileError& e)
      {
        headerLoaded = false;
      }
      
      if ( !headerLoaded ) // if not in /usr/lib/lv2, try local
      {
        try {
          imagePointer = Gdk::Pixbuf::create_from_file ("/usr/local/lib/lv2/source.lv2/header.png");
          headerLoaded = true;
        }
        catch(Glib::FileError& e)
        {
          cout << "Refractor: Header image could not be loaded! Continuing..." << e.what() << endl;
          headerLoaded = false;
          return;
        }
      }
      
      // Detect transparent colors for loaded image
      Cairo::Format format = Cairo::FORMAT_RGB24;
      if (imagePointer->get_has_alpha())
      {
          format = Cairo::FORMAT_ARGB32;
      }
      
      // Create a new ImageSurface
      imageSurfacePointer = Cairo::ImageSurface::create  (format, imagePointer->get_width(), imagePointer->get_height());
      
      // Create the new Context for the ImageSurface
      Cairo::RefPtr< Cairo::Context > imageContextPointer = Cairo::Context::create (imageSurfacePointer);
      
      // Draw the image on the new Context
      Gdk::Cairo::set_source_pixbuf (imageContextPointer, imagePointer, 0.0, 0.0);
      imageContextPointer->paint();
      
      headerLoaded = true;
    }
    
    bool on_expose_event			(GdkEventExpose* event)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      
      if(window)
      {
        Gtk::Allocation allocation = get_allocation();
        width = allocation.get_width();
        height = allocation.get_height();
        
        // clip reigon
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        cr->rectangle(event->area.x, event->area.y,
                event->area.width, event->area.height);
        cr->clip();
        
        cr->rectangle(event->area.x, event->area.y,
            event->area.width, event->area.height);
        cr->set_source_rgb(0.0,0.0,0.0);
        cr->fill();
        
        if ( headerLoaded )
        {
          cr->save();
          
          // draw header
          cr->set_source (imageSurfacePointer, 0.0, 0.0);
          cr->rectangle (0.0, 0.0, imagePointer->get_width(), imagePointer->get_height());
          cr->clip();
          cr->paint();
          
          cr->restore();
        }
        
        // loop draw the bottom 5 backgrounds
        int drawX = 33;
        int drawY = 74;
        
        // Oscillators
        Background( cr, drawX, drawY, 159, 135, "OSC 1");
        drawY += 135 + 24;
        Background( cr, drawX, drawY, 159, 135, "OSC 2");
        drawY += 135 + 24;
        Background( cr, drawX, drawY, 159, 135, "OSC 3");
        
        
        // ADSR
        drawX += 159 + 24;
        drawY = 74;
        Background( cr, drawX, drawY, 159, 135, "ADSR 1");
        drawY += 135 + 24;
        Background( cr, drawX, drawY, 159, 135, "ADSR 2");
        
        // Stepper
        drawY += 135 + 24;
        Background( cr, drawX, drawY, 159 + 24 + 159, 135, "Step Seq");
        
        
        // LFO
        drawX += 159 + 24;
        drawY = 74;
        Background( cr, drawX, drawY, 159, 135, "LFO 1");
        drawY += 135 + 24;
        Background( cr, drawX, drawY, 159, 135, "LFO 2");
        
        // Affect & Remove
        drawX += 159 + 24;
        drawY = 74;
        Background(cr, drawX, drawY, 159, 215, "Affect");
        drawY += 215 + 24;
        Background(cr, drawX, drawY, 159, 215, "Remove");
        
        // Adjust and Master
        drawX += 159 + 24;
        drawY = 74;
        Background(cr, drawX, drawY, 159, 215, "Adjust");
        drawY += 215 + 24;
        Background(cr, drawX, drawY, 159, 215, "Master");
        
        // fill in widgets
        drawMaster(cr);
        
        drawRemove(cr);
      }
      return true;
    }
    
    void Background(Cairo::RefPtr<Cairo::Context> cr, float x, float y, float sizeX, float sizeY, std::string name)
    {
      // fill background
      cr->rectangle( x, y, sizeX, sizeY);
      setColour( cr, COLOUR_GREY_3 );
      cr->fill();
      
      // set up dashed lines, 1 px off, 1 px on
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      
      // loop over each 2nd line, drawing dots
      for ( int i = x; i < x + sizeX; i += 4 )
      {
        cr->move_to( i, y );
        cr->line_to( i, y + sizeY );
      }
      
      setColour( cr, COLOUR_GREY_4, 0.5 );
      cr->stroke();
      cr->unset_dash();
      
      // draw header
      if ( true )
      {
        // backing
        cr->rectangle( x, y, sizeX, 20);
        setColour( cr, COLOUR_GREY_4 );
        cr->fill();
        
        // text
        std::string text = name;
        cr->move_to( x + 10, y + 14 );
        setColour( cr, COLOUR_BLUE_1 );
        cr->set_font_size( 10 );
        cr->show_text( text );
        
        // lower stripe
        cr->move_to( x        , y + 20 );
        cr->line_to( x + sizeX, y + 20 );
        setColour( cr, COLOUR_BLUE_1 );
        cr->stroke();
      }
      
      // stroke rim
      cr->rectangle( x, y, sizeX, sizeY);
      setColour( cr, COLOUR_BLUE_1 );
      cr->stroke();
    }
    
    void SimpleDial( Cairo::RefPtr<Cairo::Context> cr, bool active, float x, float y, float value)
    {
      int xc = x + 16;
      int yc = y + 22;
      
      float radius = 14;
      
      cr->set_line_cap( Cairo::LINE_CAP_ROUND );
      cr->set_line_join( Cairo::LINE_JOIN_ROUND);
      cr->set_line_width(2.8);
      
      // Arc Angle Value
      cr->set_line_width(2.4);
      cr->move_to(xc,yc);
      cr->set_source_rgba( 0,0,0,0 );
      cr->stroke();
      
      // main arc
      if ( active )
        setColour(cr, COLOUR_GREY_4 );
      else
        setColour(cr, COLOUR_GREY_3 );
      cr->arc(xc,yc, radius, 2.46, 0.75 );
      cr->move_to(xc,yc);
      cr->stroke();
      
      cr->set_line_width(2.8);
      float angle;
      
      if ( value < 0 )
        value = 0.f;
      
      angle = 2.46 + (4.54 * value);
      
      if ( active )
        setColour(cr, COLOUR_GREY_1 );
      else
        setColour(cr, COLOUR_GREY_2 );
      
      cr->set_line_width(1.7);
      cr->arc(xc,yc, 13, 2.46, angle );
      cr->line_to(xc,yc);
      cr->stroke();
      cr->arc(xc,yc, 17, 2.46, angle );
      cr->line_to(xc,yc);
      cr->stroke();
    }
    
    void setColour( Cairo::RefPtr<Cairo::Context> cr, Colour c, float alpha)
    {
      switch( c )
      {
        case COLOUR_ORANGE_1:
          cr->set_source_rgba( 255 / 255.f, 104 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_ORANGE_2:
          cr->set_source_rgba( 178 / 255.f,  71 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_ORANGE_3:
          cr->set_source_rgba(  89 / 255.f,  35 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_1:
          cr->set_source_rgba(  25 / 255.f, 255 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_2:
          cr->set_source_rgba(  17 / 255.f, 179 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_GREEN_3:
          cr->set_source_rgba(   8 / 255.f,  89 / 255.f ,   0 / 255.f , alpha ); break;
        case COLOUR_BLUE_1:
          cr->set_source_rgba(   0 / 255.f, 153 / 255.f , 255 / 255.f , alpha ); break;
        case COLOUR_BLUE_2:
          cr->set_source_rgba(  20 / 255.f,  73 / 255.f , 109 / 255.f , alpha ); break;
        case COLOUR_BLUE_3:
          cr->set_source_rgba(   0 / 255.f,  53 / 255.f ,  89 / 255.f , alpha ); break;
        case COLOUR_PURPLE_1:
          cr->set_source_rgba( 230 / 255.f,   0 / 255.f , 255 / 255.f , alpha ); break;
        case COLOUR_PURPLE_2:
          cr->set_source_rgba( 161 / 255.f,   0 / 255.f , 179 / 255.f , alpha ); break;
        case COLOUR_PURPLE_3:
          cr->set_source_rgba(  80 / 255.f,   0 / 255.f ,  89 / 255.f , alpha ); break;
        case COLOUR_GREY_1:
          cr->set_source_rgba( 130 / 255.f, 130 / 255.f , 130 / 255.f , alpha ); break;
        case COLOUR_GREY_2:
          cr->set_source_rgba(  98 / 255.f,  98 / 255.f ,  98 / 255.f , alpha ); break;
        case COLOUR_GREY_3:
          cr->set_source_rgba(  66 / 255.f,  66 / 255.f ,  66 / 255.f , alpha ); break;
        case COLOUR_GREY_4:
          cr->set_source_rgba(  28 / 255.f,  28 / 255.f ,  28 / 255.f , alpha ); break;
        case COLOUR_RECORD_RED:
          cr->set_source_rgba(  226 / 255.f, 0/255.f , 0/255.f, alpha ); break;
        case COLOUR_TRANSPARENT:
          cr->set_source_rgba(  0, 0, 0, 0.f ); break;
        case COLOUR_BACKGROUND: default:
          cr->set_source_rgba(  40 / 255.f,  40 / 255.f ,  40 / 255.f , alpha ); break;
      }
    }
    
    void setColour( Cairo::RefPtr<Cairo::Context> cr, Colour c)
    {
      setColour(cr, c, 1.f);
    }
    
    bool on_button_press_event(GdkEventButton* event)
    {
      /*
      // set mousedown to false first, if a hotspot is clicked, it will
      // be set to true, same with clickedWidget
      mouseDown = false;
      clickedWidget = CLICKED_WIDGET_NONE;
      
      // for calculation of mouseMove
      clickValue = 1 - ( event->y / height);
      clickValueX = 1- ( event->x / height);
      
      cout << "Refractor: Press event \tX " << event->x << "\tY " << event->y << endl;
      if ( event->button == 1 || event->button == 3 )
      {
        int x = event->x;
        int y = event->y;
        
        
        if ( x > 623 && y > 26 && y < 70 ) // OpenAV
        {
          // If GVFS isn't installed on the local machine, GTK won't know
          // how to handle 
          GError* e = NULL;
          gtk_show_uri(NULL, "http://www.openavproductions.com", GDK_CURRENT_TIME, &e);
          if ( e )
          {
            cout << "Error opening \"www.openavproductions.com\"... " << e->message << endl;
          }
          return true;
        }
        
        if ( x > 411 && y > 341 && x < 549 && y < 424 )
        {
          //cout << "Bitcrush click!" << endl;
          mouseDown = true;
          target = &guiState->bitcrush;
          clickedWidget = CLICKED_WIDGET_BITCRUSHER;
        }
        
        if ( x > 411 && y > 434 && x < 549 && y < 516 )
        {
          //cout << "Distort click!" << endl;
          mouseDown = true;
          target = &guiState->distortion;
          clickedWidget = CLICKED_WIDGET_DISTORTION;
        }
        
        if ( x > 594 && y > 341 && x < 732 && y < 424 )
        {
          //cout << "Highpass click!" << endl;
          mouseDown = true;
          target = &guiState->highpass;
          clickedWidget = CLICKED_WIDGET_HIGHPASS;
        }
        
        if ( x > 594 && y > 434 && x < 732 && y < 516 )
        {
          //cout << "Lowpass click!" << endl;
          mouseDown = true;
          target = &guiState->lowpass;
          clickedWidget = CLICKED_WIDGET_LOWPASS;
        }
        
        if ( x > 236 && y > 340 && x < 236 + 138 && y < 340 + 175 ) // RETRIGGER ZONE
        {
          // location 399, 330
          // size = 158 x 195
          
          if ( event->button == 3 )
          {
            float writeVal;
            for (int i = 0; i < 16; i++)
            {
              
              // can't use a control port like this, only the "latest" value
              // is accepted, so the others aren't affected. Need to look at using
              // a custom "reset" Atom event. 
              guiState->retriggerArray[i] = 0;
              redraw(220, 340, 200, 500 );
              writeVal = int(i);
              guiState->write_function( guiState->controller, REFRACTOR_CONTROL_RETRIGGER, sizeof(float), 0, (const void*)&writeVal );
            }
            return true;
          }
          
          int border = 10;
          int x = 216 + border;
          int y = 330 + border;
          
          int xSize = 158 - 2 * border;
          int ySize = 195 - 2 * border;
          
          int slice = (event->y - y) / (ySize / 16.f);
          int retrg = (event->x - x) / (xSize /  5.f);
          
          // write value into gui state
          guiState->retriggerArray[slice] = retrg;
          redraw(x,y,xSize,ySize);
          
          // multiplex slice & retrigger into the one float variable, to send to DSP
          float value = slice + (retrg / 10.f);
          //cout << "Retrigger slice " << slice << ", retrig " << retrg << "  value " << value << endl;
          
          guiState->write_function( guiState->controller, REFRACTOR_CONTROL_RETRIGGER, sizeof(float), 0, (const void*)&value);
        }
        
        if ( x > 859 && y > 341 && x < 917 && y < 516 )
        {
          if ( event->button == 3 )
          {
            guiState->masterVol = 0.7;
            redraw(859, 341, 200, 500 );
            guiState->write_function( guiState->controller, CLICKED_WIDGET_MASTER_VOLUME, sizeof(float), 0, (const void*)&guiState->masterVol);
            return true;
          }
          clickedWidget = CLICKED_WIDGET_MASTER_VOLUME;
          mouseDown = true;
          target = &guiState->masterVol;
        }
        
        if ( x > 127 && y > 341 && x < 185 && y < 516 )
        {
          if ( event->button == 3 )
          {
            guiState->pitch = 0.5;
            redraw(127, 185, 200, 500 );
            guiState->write_function( guiState->controller, CLICKED_WIDGET_PITCH, sizeof(float), 0, (const void*)&guiState->pitch);
            return true;
          }
          
          clickedWidget = CLICKED_WIDGET_PITCH;
          mouseDown = true;
          target = &guiState->pitch;
        }
        
      }
      */
    }
    
    bool on_button_release_event(GdkEventButton* event)
    {
    }
    
    bool on_mouse_move(GdkEventMotion* event)
    {
      /*
      // if mouse moves over "openavproductions.com", show "link" cursor
      if ( event->x > 623 && event->y > 20 && event->y < 70 )
      {
        Glib::RefPtr <Gdk::Window> ref_window;
        ref_window = get_window();
        Gdk::Cursor m_Cursor(Gdk::HAND1);
        ref_window->set_cursor(m_Cursor);
      }
      else
      {
        Glib::RefPtr <Gdk::Window> ref_window;
        ref_window = get_window();
        ref_window->set_cursor();
      }
      
      // when a click occurs, float* target is set to that location,
      // here we just work with target
      if ( mouseDown && target && clickedWidget != CLICKED_WIDGET_NONE )
      {
        float click  = 1 - (event->y / height);
        float clickX = 1 - (event->x / height);
        
        float delta = clickValue - click;
        
        bool invert = false;
        
        if ( clickedWidget == CLICKED_WIDGET_HIGHPASS )
        {
          // use X delta, not Y
          delta = clickValueX - clickX;
          invert = true;
          //cout << "Highpass delta = " << delta << endl;
        }
        else if ( clickedWidget == CLICKED_WIDGET_LOWPASS  )
        {
          // use X delta, not Y
          delta = clickValueX - clickX;
          invert = true;
          //cout << "Lowpass delta = " << delta << endl;
        }
        
        clickValue  = 1 - ( event->y / height); // reset the starting point, no acceleration 
        clickValueX = 1 - ( event->x / height); // reset the starting point, no acceleration 
        
        //cout << "Delta = " << click << endl;
        
        // scale delta to make it have more effect
        float value = (*target) - delta * 3;
        
        if ( invert )
        {
          value = (*target) + delta * 3;
        }
        
        if ( value > 1.f) value = 1.f;
        if ( value < 0.f) value = 0.f;
        
        if ( clickedWidget == REFRACTOR_CONTROL_RETRIGGER )
        {
          guiState->retrigger = value;
          value = int(value * 16.f);
          redraw(227,447, 40, 40);
        }
        else if ( clickedWidget == REFRACTOR_MASTER_VOLUME )
        {
          guiState->masterVol = value;
          redraw( 859, 340, 60, 190);
        }
        else if ( clickedWidget == REFRACTOR_CONTROL_BITCRUSH )
        {
          guiState->bitcrush = value;
          redraw( 406,330, 150, 100);
        }
        else if ( clickedWidget == REFRACTOR_CONTROL_DISTORTION )
        {
          guiState->distortion = value;
          redraw( 406,430, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_HIGHPASS )
        {
          guiState->highpass = value;
          redraw( 594,330, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_LOWPASS )
        {
          guiState->lowpass = value;
          redraw( 594,430, 150, 100);
        }
        else if ( clickedWidget == CLICKED_WIDGET_PITCH )
        {
          //cout << "Setting & drawing pitch / source" << endl;
          guiState->pitch = value;
          redraw( 127, 340, 70, 200);
        }
        //cout << "Writing value " << value << " scaled to " << *target << " to port index" << clickedWidget << endl;
        guiState->write_function( guiState->controller, clickedWidget, sizeof(float), 0, (const void*)&value);
      }
      */
    }
    
};


#endif
