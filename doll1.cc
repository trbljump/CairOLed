#include <cairo.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <Box2D.h>

#include <iostream>
#include <vector>
#include <map>

#include "hooman.h"




#ifdef USE_X11
#include "cairo_x.h"
#else
#include "cairo_fb.h"
#endif /* USE_X11 */

static b2World* createWorld(double width, double height) {
  b2World* myWorld ;

  b2Vec2 vs[3] ;
  b2ChainShape chain;
  b2BodyDef groundBodyDef ;
  b2FixtureDef groundFixtureDef;
  b2Body* groundBody ;

  myWorld = new b2World(b2Vec2(0, -9.81)) ;
  
  vs[0].Set(0, height) ;
  vs[1].Set(0, 0);
  vs[2].Set(1000*width, 0);
  chain.CreateChain(vs, 3);
  
  groundBodyDef.position.Set(0.0f, 0.0f) ;
  groundBodyDef.linearDamping=0;
  groundBodyDef.angularDamping=0;
  groundFixtureDef.shape = &chain ;
  groundFixtureDef.friction=0.01;
  groundFixtureDef.restitution=0.1;
  groundBody = myWorld->CreateBody(&groundBodyDef);
  groundBody->CreateFixture(&groundFixtureDef) ;

  return myWorld ;
}

static cairo_t* createCairo(double width, double height) {
  cairo_surface_t* surface;
  cairo_t *cr;
  double x1,x2,y1,y2 ;
  cairo_matrix_t matrix ;
  double scale, xs, ys ;
  double ox, oy ;

#ifdef USE_X11
  surface = cairo_create_x11_surface(720, 480);
#else
  surface = cairo_linuxfb_surface_create("/dev/fb1");
#endif
  
  cr = cairo_create(surface) ;
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2) ;

  xs = (x2-y1)/width ;
  ys = (y2-y1)/height ;

  scale = xs > ys ? ys : xs ;
  ox = (((x2-x1)/scale)-width)/2 ;
  oy = (((y2-y1)/scale)-height)/2 ;

  cairo_matrix_init_scale(&matrix, scale, scale) ;
  cairo_matrix_translate(&matrix, ox, oy) ;
  cairo_set_matrix(cr, &matrix) ;

#ifdef USE_X11
  cairo_set_line_width(cr, 1.5/scale) ;
#else
  cairo_set_line_width(cr, 1.0/scale) ;
#endif

  cairo_select_font_face(cr, "Lato",
			 CAIRO_FONT_SLANT_NORMAL,
			 CAIRO_FONT_WEIGHT_BOLD);
  
  cairo_set_font_size(cr, 0.65);
  
  return cr ;
}

int main(int argc, char **argv) {
  cairo_t* cr ;
  b2World* world ;
  cairo_matrix_t matrix ;  
  std::vector<Hooman*> hoomans ;
  double width, height ;
  double xTranslation ;

  height = 2.5 ;
  width = 1.5*height ;
  xTranslation=0 ;
  
  cr = createCairo(width, height) ;
  world = createWorld(width, height) ;

  cairo_matrix_init_scale(&matrix, 1, -1) ;
  cairo_matrix_translate(&matrix, 0, -height) ;
  
  while (1) {
    double averageX ;
    size_t i ;
    cairo_matrix_t tMatrix ;  
    
    while (hoomans.size() < 5) {
      hoomans.push_back(new Hooman(*world)) ;
    }

    world->Step((1.0/25), 8, 8) ;

    for (i=0,averageX=0 ; i < hoomans.size() ; i++) {
      averageX += hoomans[i]->getX() ;
    }

    averageX = (averageX/i)-width/2 ;
    if (averageX < 0) {
      averageX = 0 ;
    }

    cairo_save(cr) ;

    xTranslation = 0.05*averageX + 0.95*xTranslation ;
    cairo_matrix_init_translate(&tMatrix, -xTranslation, 0) ;
    cairo_transform(cr, &tMatrix) ;

    cairo_set_source_rgb (cr, 0, 0, 0) ;
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5) ;
    cairo_move_to(cr, 0.12, 2);
    
    cairo_show_text(cr, "Ragdolls!") ;

    cairo_save(cr) ;
    cairo_transform(cr, &matrix) ;

    for (auto i = hoomans.begin() ;
	 i != hoomans.end() ;
	 ++i) {
      (*i)->render_to_cairo(cr) ;

      if (drand48() < 0.1) {
	size_t n ;
	n = (*i)->variableCount() ;
	(*i)->setVariable(drand48()*n, 5*drand48()-2.5) ;
      }
    }
    
    cairo_restore(cr) ;

    cairo_surface_flush(cairo_get_target(cr)) ;
#ifdef USE_X11
    cairo_flush_x11_surface(cairo_get_target(cr)) ;
#endif

    for (auto j=hoomans.begin() ;
	 j != hoomans.end() ;
	 ++j) {
      if (! (*j)->isAlive()) {
	delete *j ;
	hoomans.erase(j) ;
	if (j == hoomans.end()) {
	  break ;
	}
      }
    }
    
    cairo_restore(cr) ;
    usleep(2*1000000/30);
  }

  for (auto i = hoomans.begin() ;
       i != hoomans.end() ;
       ++i) {
    delete *i ;
  }

  delete world ;
  cairo_surface_destroy(cairo_get_target(cr));
  cairo_destroy(cr);

  return 0;
}

