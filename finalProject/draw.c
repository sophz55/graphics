#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "gmath.h"

/*======== void add_polygon() ==========
Inputs:   struct matrix *surfaces
         double x0
         double y0
         double z0
         double x1
         double y1
         double z1
         double x2
         double y2
         double z2  
Returns: 
Adds the vertices (x0, y0, z0), (x1, y1, z1)
and (x2, y2, z2) to the polygon matrix. They
define a single triangle surface.

04/16/13 13:05:59
jdyrlandweaver
====================*/
void add_polygon( struct matrix *polygons, 
		  double x0, double y0, double z0, 
		  double x1, double y1, double z1, 
		  double x2, double y2, double z2 ) {
  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
Inputs:   struct matrix *polygons
          screen s
          color c  
Returns: 
Goes through polygons 3 points at a time, drawing 
lines connecting each points to create bounding
triangles

04/16/13 13:13:27
jdyrlandweaver
====================*/
void draw_polygons( struct matrix *polygons, screen s, color c, struct matrix *zb, light light_source) {
  
  int i;  
  /*
  */
  //SCANLINE CONVERT STARTS HERE
  for( i=0; i < polygons->lastcol-2; i+=3 ) {
    
    if ( calculate_dot( polygons, i ) < 0 ) {
      //      c.red = rand() %255;
      //      c.green = rand() %255;
      //      c.blue = rand() %255;

      int x0, x1, y, 
	  yb, ym, yt, 
	  xb, xm, xt,
	  b, m, t;

      double z0, z1, zbot, zm, zt;

      //find B, M, T
      if ( polygons->m[1][i] < polygons->m[1][i+1]){
	if ( polygons->m[1][i] < polygons->m[1][i+2]){
	  b = i;
	  if ( polygons->m[1][i+1] < polygons->m[1][i+2]){
	    m = i + 1;
	    t = i + 2;
	  }
	  else {
	    m = i + 2;
	    t = i + 1;
	  }
	}
	else {
	  b = i + 2;
	  m = i;
	  t = i + 1;
	}
      }
      else {
	if ( polygons->m[1][i+1] < polygons->m[1][i+2]){
	  b = i + 1;
	  if ( polygons->m[1][i] < polygons->m[1][i+2]){
	    m = i;
	    t = i + 2;
	  }
	  else {
	    m = i + 2;
	    t = i;
	  }
	}
	else {
	  b = i + 2;
	  m = i + 1;
	  t = i;
	}
      }      

      xb = polygons->m[0][b];
      yb = polygons->m[1][b];
      xm = polygons->m[0][m];
      ym = polygons->m[1][m];
      xt = polygons->m[0][t];
      yt = polygons->m[1][t];

      zbot = polygons->m[2][b];
      zm = polygons->m[2][m];
      zt = polygons->m[2][t];

      printf( "x: %i, y: %i, z: %f \n", xb, yb, zbot);
      
      /* START HERE FOR FLAT SHADING? */
      /*        //brightness
      float red = c.light_brightness;
      float green = c.light_brightness;
      float blue = c.light_brightness;
      */
      float red = 0;
      float green = 0;
      float blue = 0;
      
      double ax = xt - xm;
      double ay = yt - ym;
      double az = zt - zm;
      double bx = xm - xb;
      double by = ym - yb;
      double bz = zm - zbot;
      
      double* normal = calculate_normal( ax, ay, az, bx, by, bz );
      printf( "normal-x: %i\nnormal-y: %i\nnormal-z: %i\n", normal[0], normal[1], normal[2] );
      double normal_magnitude = sqrt( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] ); 

      //specular
      double vx = xt - light_source.x;
      double vy = xt - light_source.y;
      double vz = xt - light_source.z;
      //printf("light source x: %i\n lightsource y: %i\n lightsource z: %i\n", light_source.x, light_source.y, light_source.z );
      printf("vx: %f\n vy: %f\n vz: %f\n", vx, vy, vz );
      double specular_magnitude = sqrt( vx * vx + vy * vy + vz * vz );
      printf("specular_magnitude: %f\n", specular_magnitude );
      double dot_prod = normal[0] * vx + normal[1] * vy + normal[2] * vz;
      double divide = specular_magnitude * normal_magnitude;

      double diffuse = dot_prod / divide;

      normal[0] = normal[0]/normal_magnitude;
      normal[1] = normal[1]/normal_magnitude;
      normal[2] = normal[2]/normal_magnitude;
      //      double nmag = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
      
      vx = vx/specular_magnitude;
      vy = vy/specular_magnitude;
      vz = vz/specular_magnitude;
      specular_magnitude = sqrt(vx * vx + vy * vy + vz * vz);

      //calculate dot product between specular vector and surface norm
      double normal_dot = normal[0] * vx + normal[1] * vy + normal[2] * vz;
	
      //calc reflection vector
      int reflection[3];
      reflection[0] = vx - 2 * normal_dot * normal[0];
      reflection[1] = vy - 2 * normal_dot * normal[1];
      reflection[2] = vz - 2 * normal_dot * normal[2];

      //calculate dot product between reflection and view vector [0,0,1]
      double specdot = reflection[0] * 0 + reflection[1] * 0 + reflection[2] * -1;
      divide = normal_magnitude * specular_magnitude * 4;
      double specular = specdot / divide;
      
      double new_color_mult = (diffuse + specular ) / 2;
      if ( new_color_mult >= 1 )
	new_color_mult = 1;
      printf( "mags: %f, new color mult: %f\n", specular_magnitude, new_color_mult );

      //ambient
      red = c.red * c.ambient_k;
      green = c.green * c.ambient_k;
      blue = c.blue * c.ambient_k;

      printf( "ambient_k: %f\n", c.ambient_k);
      printf( "specular_k: %f\n", c.specular_k);
      /*
      printf( "red: %f\n", red );
      printf( "green: %f\n", green );
      printf( "init blue: %f\n\n\n\n", blue );
      */
      //diffuse and specular
      if( new_color_mult > 0 ) {
	red += new_color_mult * c.specular_k;
	green += new_color_mult * c.specular_k;
	blue += new_color_mult * c.specular_k;
	printf("ran?\n");
      }
      /*
      printf( "c.red: %i\n", c.red );
      printf( "c.green: %i\n", c.green );
      printf( "c.blue: %i\n\n\n\n", c.blue );
      */
      c.red += red;
      c.green += green;
      c.blue += blue;
      /*    
      printf( "red: %i\n", red );
      printf( "green: %i\n", green );
      printf( "blue: %i\n", blue ); 
      */
      printf( "red: %i\n", c.red);
      printf( "green: %i\n", c.green );
      printf( "blue: %i\n\n\n\n", c.blue );

      //don't go over max color
      if( c.red > MAX_COLOR )
	c.red = MAX_COLOR;
      if( c.blue > MAX_COLOR )
	c.blue = MAX_COLOR;
      if( c.green > MAX_COLOR )
	c.green = MAX_COLOR;
      
      /* DONT DESTROY ANY MORE*/
      
      if (yb != yt) {
	double delt0, delt1, doubx0, doubx1;
	double zdelt0, zdelt1;
	x0 = xb;
	x1 = xb;
	z0 = zbot;
	z1 = zbot;
	if (yb == ym) {
	  x1 = xm;
	  z1 = zm;
	}

	doubx0 = (double)x0;
	doubx1 = (double)x1;

	for (y = yb; y < yt; y++){
	  delt0 = ((double)xt - xb) / (yt - yb);
	  zdelt0 = (zt - zbot) / (yt - yb);
	  if (y < ym) {
	    delt1 = ((double)xm - xb) / (ym - yb);
	    zdelt1 = (zm - zbot) / (ym - yb);
	    draw_line( x0, y, z0, x1, y, z1, s, c, zb );
	  }	
	  else {
	    delt1 = ((double)xt - xm) / (yt - ym);
	    zdelt1 = (zt - zm) / (yt - ym);
	    draw_line( x0, y, z0, x1, y, z1, s, c, zb );
	  }	
	  doubx0 += delt0;
	  doubx1 += delt1;
	  x0 = (int)doubx0;
	  x1 = (int)doubx1;
	  z0 += zdelt0;
	  z1 += zdelt1;

	}
      }
      
      //border
      draw_line( polygons->m[0][i],
		 polygons->m[1][i],
		 polygons->m[2][i],
		 polygons->m[0][i+1],
		 polygons->m[1][i+1],
		 polygons->m[2][i+1],
		 s, c, zb);
      draw_line( polygons->m[0][i+1],
		 polygons->m[1][i+1],
		 polygons->m[2][i+1],
		 polygons->m[0][i+2],
		 polygons->m[1][i+2],
		 polygons->m[2][i+2],
		 s, c, zb);
      draw_line( polygons->m[0][i+2],
		 polygons->m[1][i+2],
		 polygons->m[2][i+2],
		 polygons->m[0][i],
		 polygons->m[1][i],
		 polygons->m[2][i],
		 s, c, zb);
    }
  }
}


/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double r
	    double step  
  Returns: 

  adds all the points for a sphere with center 
  (cx, cy) and radius r.

  should call generate_sphere to create the
  necessary points

  jdyrlandweaver
  ====================*/
void add_sphere( struct matrix * points, 
		 double cx, double cy, double cz, double r, 
		 int step ) {

  struct matrix * temp;
  int lat, longt;
  int index;
  int num_steps, num_points;
  double px0, px1, px2, px3;
  double py0, py1, py2, py3;
  double pz0, pz1, pz2, pz3;

  num_steps = MAX_STEPS / step;
  num_points = num_steps * (num_steps + 1);
  
  temp = new_matrix( 4, num_points);
  //generate the points on the sphere
  generate_sphere( temp, cx, cy, cz, r, step );

  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = num_steps;
  longStart = 0;
  longStop = num_steps;

  num_steps++;

  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {
      
      index = lat * num_steps + longt;

      px0 = temp->m[0][ index ];
      py0 = temp->m[1][ index ];
      pz0 = temp->m[2][ index ];
      
      px1 = temp->m[0][ (index + num_steps) % num_points ];
      py1 = temp->m[1][ (index + num_steps) % num_points ];
      pz1 = temp->m[2][ (index + num_steps) % num_points ];

      px3 = temp->m[0][ index + 1 ];
      py3 = temp->m[1][ index + 1 ];
      pz3 = temp->m[2][ index + 1 ];

      if (longt != longStop - 1) {
	px2 = temp->m[0][ (index + num_steps + 1) % num_points ];
	py2 = temp->m[1][ (index + num_steps + 1) % num_points ];
	pz2 = temp->m[2][ (index + num_steps + 1) % num_points ];
      }
      else {
	px2 = temp->m[0][ (index + 1) % num_points ];
	py2 = temp->m[1][ (index + 1) % num_points ];
	pz2 = temp->m[2][ (index + 1) % num_points ];
      }

      if (longt != 0)
	add_polygon( points, px0, py0, pz0, px1, py1, pz1, px2, py2, pz2 );
      if (longt != longStop - 1)
	add_polygon( points, px2, py2, pz2, px3, py3, pz3, px0, py0, pz0 );
    }
  }
}

/*======== void generate_sphere() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double r
	    double step  
  Returns: 

  Generates all the points along the surface of a 
  sphere with center (cx, cy) and radius r

  Adds these points to the matrix parameter

  03/22/12 11:30:26
  jdyrlandweaver
  ====================*/
void generate_sphere( struct matrix * points, 
		      double cx, double cy, double cz, double r, 
		      int step ) {


  int circle, rotation;
  double x, y, z, circ, rot;

  int rotStart = step * 0;
  int rotStop = MAX_STEPS;
  int circStart = step * 0;
  int circStop = MAX_STEPS;
  
  for ( rotation = rotStart; rotation < rotStop; rotation += step ) {
    rot = (double)rotation / MAX_STEPS;
    for ( circle = circStart; circle <= circStop; circle+= step ) {

      circ = (double)circle / MAX_STEPS;
      x = r * cos( M_PI * circ ) + cx;
      y = r * sin( M_PI * circ ) *
	cos( 2 * M_PI * rot ) + cy;
      z = r * sin( M_PI * circ ) *
	sin( 2 * M_PI * rot ) + cz;

      add_point( points, x, y, z);
    }
  }
}    


/*======== void add_torus() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double r1
	    double r2
	    double step  
  Returns: 

  adds all the points required to make a torus
  with center (cx, cy) and radii r1 and r2.

  should call generate_torus to create the
  necessary points

  03/22/12 13:34:03
  jdyrlandweaver
  ====================*/
void add_torus( struct matrix * points, 
		double cx, double cy, double cz, double r1, double r2, 
		int step ) {

  struct matrix * temp;
  int lat, longt;
  int index;
  int num_steps;
  
  num_steps = MAX_STEPS / step;

  temp = new_matrix( 4, num_steps * num_steps );
  //generate the points on the torus
  generate_torus( temp, cx, cy, cz, r1, r2, step );
  int num_points = temp->lastcol;

  int latStop, longtStop, latStart, longStart;
  latStart = 0;
  longStart = 0;
  latStop = num_steps;
  longtStop = num_steps;
  for ( lat = latStart; lat < latStop; lat++ )
    for ( longt = longStart; longt < longtStop; longt++ ) {

      index = lat * num_steps + longt;

      if ( longt != num_steps-1) {
	add_polygon( points, temp->m[0][index],
		     temp->m[1][index],
		     temp->m[2][index],
		     temp->m[0][(index+num_steps+1) % num_points],
		     temp->m[1][(index+num_steps+1) % num_points],
		     temp->m[2][(index+num_steps+1) % num_points],
		     temp->m[0][index+1],
		     temp->m[1][index+1],
		     temp->m[2][index+1] );
	add_polygon( points, temp->m[0][index],
		     temp->m[1][index],
		     temp->m[2][index],
		     temp->m[0][(index+num_steps) % num_points],
		     temp->m[1][(index+num_steps) % num_points],
		     temp->m[2][(index+num_steps) % num_points],
		     temp->m[0][(index+num_steps) % num_points + 1],
		     temp->m[1][(index+num_steps) % num_points + 1],
		     temp->m[2][(index+num_steps) % num_points + 1]);
      }
      else {
	add_polygon( points, temp->m[0][index],
		     temp->m[1][index],
		     temp->m[2][index],
		     temp->m[0][(index+1) % num_points],
		     temp->m[1][(index+1) % num_points],
		     temp->m[2][(index+1) % num_points],
		     temp->m[0][index+1-num_steps],
		     temp->m[1][index+1-num_steps],
		     temp->m[2][index+1-num_steps] );
	add_polygon( points, temp->m[0][index],
		     temp->m[1][index],
		     temp->m[2][index],
		     temp->m[0][(index+num_steps) % num_points],
		     temp->m[1][(index+num_steps) % num_points],
		     temp->m[2][(index+num_steps) % num_points],
		     temp->m[0][(index+1) % num_points],
		     temp->m[1][(index+1) % num_points],
		     temp->m[2][(index+1) % num_points]);
      }

    }
}

/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double r
	    double step  
  Returns: 

  Generates all the points along the surface of a 
  tarus with center (cx, cy) and radii r1 and r2

  Adds these points to the matrix parameter

  03/22/12 11:30:26
  jdyrlandweaver
  ====================*/
void generate_torus( struct matrix * points, 
		     double cx, double cy, double cz, double r1, double r2, 
		     int step ) {

  double x, y, z, circ, rot;
  int circle, rotation;

  double rotStart = step * 0;
  double rotStop = MAX_STEPS;
  double circStart = step * 0;
  double circStop = MAX_STEPS;

  for ( rotation = rotStart; rotation < rotStop; rotation += step ) {

    rot = (double)rotation / MAX_STEPS;
    for ( circle = circStart; circle < circStop; circle+= step ) {

      circ = (double)circle / MAX_STEPS;
      x = cos( 2 * M_PI * rot ) *
	( r1 * cos( 2 * M_PI * circ ) + r2 ) + cx;
      y = r1 * sin( 2 * M_PI * circ ) + cy;
      z = sin( 2 * M_PI * rot ) *
	( r1 * cos( 2 * M_PI * circ ) + r2 ) + cz;

      add_point( points, x, y, z );
    }
  }
}

/*======== void add_box() ==========
  Inputs:   struct matrix * points
            double x
	    double y
	    double z
	    double width
	    double height
	    double depth
  Returns: 

  add the points for a rectagular prism whose 
  upper-left corner is (x, y, z) with width, 
  height and depth dimensions.

  jdyrlandweaver
  ====================*/
void add_box( struct matrix * polygons,
	      double x, double y, double z,
	      double width, double height, double depth ) {

  double x2, y2, z2;
  x2 = x + width;
  y2 = y - height;
  z2 = z - depth;
  //front
  add_polygon( polygons, 
	       x, y, z, 
	       x, y2, z,
	       x2, y2, z);
  add_polygon( polygons, 
	       x2, y2, z, 
	       x2, y, z,
	       x, y, z);
  //back
  add_polygon( polygons, 
	       x2, y, z2, 
	       x2, y2, z2,
	       x, y2, z2);
  add_polygon( polygons, 
	       x, y2, z2, 
	       x, y, z2,
	       x2, y, z2);
  //top
  add_polygon( polygons, 
	       x, y, z2, 
	       x, y, z,
	       x2, y, z);
  add_polygon( polygons, 
	       x2, y, z, 
	       x2, y, z2,
	       x, y, z2);
  //bottom
  add_polygon( polygons, 
	       x2, y2, z2, 
	       x2, y2, z,
	       x, y2, z);
  add_polygon( polygons, 
	       x, y2, z, 
	       x, y2, z2,
	       x2, y2, z2);
  //right side
  add_polygon( polygons, 
	       x2, y, z, 
	       x2, y2, z,
	       x2, y2, z2);
  add_polygon( polygons, 
	       x2, y2, z2, 
	       x2, y, z2,
	       x2, y, z);
  //left side
  add_polygon( polygons, 
	       x, y, z2, 
	       x, y2, z2,
	       x, y2, z);
  add_polygon( polygons, 
	       x, y2, z, 
	       x, y, z,
	       x, y, z2); 
}
  
/*======== void add_circle() ==========
  Inputs:   struct matrix * points
            double cx
	    double cy
	    double y
	    double step  
  Returns: 


  03/16/12 19:53:52
  jdyrlandweaver
  ====================*/
void add_circle( struct matrix * points, 
		 double cx, double cy, 
		 double r, double step ) {
  
  double x0, y0, x, y, t;
  
  x0 = cx + r;
  y0 = cy;

  for ( t = step; t <= 1; t+= step ) {
    
    x = r * cos( 2 * M_PI * t ) + cx;
    y = r * sin( 2 * M_PI * t ) + cy;
    
    add_edge( points, x0, y0, 0, x, y, 0 );
    x0 = x;
    y0 = y;
  }

  add_edge( points, x0, y0, 0, cx + r, cy, 0 );
}

/*======== void add_curve() ==========
Inputs:   struct matrix *points
         double x0
         double y0
         double x1
         double y1
         double x2
         double y2
         double x3
         double y3
         double step
         int type  
Returns: 

Adds the curve bounded by the 4 points passsed as parameters
of type specified in type (see matrix.h for curve type constants)
to the matrix points

03/16/12 15:24:25
jdyrlandweaver
====================*/
void add_curve( struct matrix *points, 
		double x0, double y0, 
		double x1, double y1, 
		double x2, double y2, 
		double x3, double y3, 
		double step, int type ) {

  double x, y, t;
  struct matrix * xcoefs;
  struct matrix * ycoefs;
  
  //generate the coeficients
  if ( type == BEZIER_MODE ) {
    ycoefs = generate_curve_coefs(y0, y1, y2, y3, BEZIER_MODE);
    xcoefs = generate_curve_coefs(x0, x1, x2, x3, BEZIER_MODE);
  }

  else {
    xcoefs = generate_curve_coefs(x0, x1, x2, x3, HERMITE_MODE);
    ycoefs = generate_curve_coefs(y0, y1, y2, y3, HERMITE_MODE);
  }

  /*
  printf("a = %lf b = %lf c = %lf d = %lf\n", xcoefs->m[0][0],
         xcoefs->m[1][0], xcoefs->m[2][0], xcoefs->m[3][0]);
  */

  for (t=step; t <= 1; t+= step) {
    
    x = xcoefs->m[0][0] * t * t * t + xcoefs->m[1][0] * t * t
      + xcoefs->m[2][0] * t + xcoefs->m[3][0];

    y = ycoefs->m[0][0] * t * t * t + ycoefs->m[1][0] * t * t
      + ycoefs->m[2][0] * t + ycoefs->m[3][0];

    add_edge(points, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}

/*======== void add_point() ==========
Inputs:   struct matrix * points
         int x
         int y
         int z 
Returns: 
adds point (x, y, z) to points and increment points.lastcol
if points is full, should call grow on points
====================*/
void add_point( struct matrix * points, double x, double y, double z) {
  
  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );

  points->m[0][points->lastcol] = x;
  points->m[1][points->lastcol] = y;
  points->m[2][points->lastcol] = z;
  points->m[3][points->lastcol] = 1;

  points->lastcol++;
}

/*======== void add_edge() ==========
Inputs:   struct matrix * points
          int x0, int y0, int z0, int x1, int y1, int z1
Returns: 
add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
should use add_point
====================*/
void add_edge( struct matrix * points, 
	       double x0, double y0, double z0, 
	       double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
Inputs:   struct matrix * points
         screen s
         color c 
Returns: 
Go through points 2 at a time and call draw_line to add that line
to the screen
====================*/
void draw_lines( struct matrix * points, screen s, color c, struct matrix *zb) {

  int i;
 
  if ( points->lastcol < 2 ) {
    
    printf("Need at least 2 points to draw a line!\n");
    return;
  }

  for ( i = 0; i < points->lastcol - 1; i+=2 ) {

    draw_line( points->m[0][i], points->m[1][i], points->m[2][i], 
	       points->m[0][i+1], points->m[1][i+1], points->m[2][i+1], s, c, zb);
    //FOR DEMONSTRATION PURPOSES ONLY
    //draw extra pixels so points can actually be seen    
    /*
    draw_line( points->m[0][i]+1, points->m[1][i], 
	       points->m[0][i+1]+1, points->m[1][i+1], s, c);
    draw_line( points->m[0][i], points->m[1][i]+1, 
	       points->m[0][i+1], points->m[1][i+1]+1, s, c);
    draw_line( points->m[0][i]-1, points->m[1][i], 
	       points->m[0][i+1]-1, points->m[1][i+1], s, c);
    draw_line( points->m[0][i], points->m[1][i]-1, 
	       points->m[0][i+1], points->m[1][i+1]-1, s, c);
    draw_line( points->m[0][i]+1, points->m[1][i]+1, 
	       points->m[0][i+1]+1, points->m[1][i+1]+1, s, c);
    draw_line( points->m[0][i]-1, points->m[1][i]+1, 
	       points->m[0][i+1]-1, points->m[1][i+1]+1, s, c);
    draw_line( points->m[0][i]-1, points->m[1][i]-1, 
	       points->m[0][i+1]-1, points->m[1][i+1]-1, s, c);
    draw_line( points->m[0][i]+1, points->m[1][i]-1, 
	       points->m[0][i+1]+1, points->m[1][i+1]-1, s, c);
    */
  } 	       
}


void draw_line(int x0, int y0, double z0, int x1, int y1, double z1, screen s, color c, struct matrix * zb) {
 
  int x, y, d, dx, dy;
  double z, dzx, dzy;

  x = x0;
  y = y0;
  z = z0;
  
  //swap points so we're always drawing left to right
  if ( x0 > x1 ) {
    x = x1;
    y = y1;
    z = z1;
    x1 = x0;
    y1 = y0;
    z1 = z0;
  }

  //need to know dx and dy for this version
  dx = x1 - x;
  dy = y1 - y;

  dzx = (z1 - z) / abs(x1 - x) ;
  dzy = (z1 - z) / abs(y1 - y) ;

  dx *= 2;
  dy *= 2;

  //positive slope: Octants 1, 2 (5 and 6)
  if ( dy > 0 ) {
    
    //slope < 1: Octant 1 (5)
    if ( dx > dy ) {
      d = dy - ( dx / 2 );
      
      while ( x <= x1 ) {
	plot(s, c, x, y, z, zb);
	
	if ( d < 0 ) {
	  x = x + 1;
	  d = d + dy;
	}
	else {
	  x = x + 1;
	  y = y + 1;
	  d = d + dy - dx;
	}
	z += dzx;	
      }
    }
    
    //slope > 1: Octant 2 (6)
    else {
      d = ( dy / 2 ) - dx;
      while ( y <= y1 ) {

	plot(s, c, x, y, z, zb);
	if ( d > 0 ) {
	  y = y + 1;
	  d = d - dx;
	}
	else {
	  y = y + 1;
	  x = x + 1;
	  d = d + dy - dx;
	}
	z += dzy;	
      }
    }
  }

  //negative slope: Octants 7, 8 (3 and 4)
  else { 

    //slope > -1: Octant 8 (4)
    if ( dx > abs(dy) ) {

      d = dy + ( dx / 2 );
  
      while ( x <= x1 ) {

	plot(s, c, x, y, z, zb);

	if ( d > 0 ) {
	  x = x + 1;
	  d = d + dy;
	}
	else {
	  x = x + 1;
	  y = y - 1;
	  d = d + dy + dx;
	}
	z += dzx;
      }
    }

    //slope < -1: Octant 7 (3)
    else {

      d =  (dy / 2) + dx;

      while ( y >= y1 ) {
	
	plot(s, c, x, y, z, zb );
	if ( d < 0 ) {
	  y = y - 1;
	  d = d + dx;
	}
	else {
	  y = y - 1;
	  x = x + 1;
	  d = d + dy + dx;
	}
	z += dzy;
      }
    }
  }
  //  printf( "z %lf, z0 %lf, z1 %lf\n", z, z0, z1);
}

