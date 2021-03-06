//
//  Copyright (c) 2011-2013 Vladimir Chalupecky
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.

#include "Exact_adaptive_kernel.h"
#include "Predicates.h"

void exactinit(void);

namespace
{
  struct InitializePredicates
  {
    InitializePredicates() { exactinit(); std::clog << "Exact adaptive predicates initialized\n"; }
  };

  InitializePredicates init_predicates;
}

namespace umeshu
{

Oriented_side Exact_adaptive_kernel::oriented_side( Point2 const& pa, Point2 const& pb, Point2 const& test )
{
  double r = orient2d( pa.data(), pb.data(), test.data() );

  if ( r > 0.0 )
  {
    return ON_POSITIVE_SIDE;
  }

  if ( r < 0.0 )
  {
    return ON_NEGATIVE_SIDE;
  }

  return ON_ORIENTED_BOUNDARY;
}

Oriented_side Exact_adaptive_kernel::oriented_circle( Point2 const& pa, Point2 const& pb, Point2 const& pc, Point2 const& test )
{
  double r = incircle( pa.data(), pb.data(), pc.data(), test.data() );

  if ( r > 0.0 )
  {
    return ON_POSITIVE_SIDE;
  }

  if ( r < 0.0 )
  {
    return ON_NEGATIVE_SIDE;
  }

  return ON_ORIENTED_BOUNDARY;
}

Point2 Exact_adaptive_kernel::circumcenter( Point2 const& a, Point2 const& b, Point2 const& c )
{
  Point2 ba = b - a;
  Point2 ca = c - a;
  double bal = distance_squared( a, b );
  double cal = distance_squared( a, c );
  double denominator = 0.25 / signed_area( b, c, a );
  Point2 d( ( ca( 1 ) * bal - ba( 1 ) * cal ) * denominator, ( ba( 0 ) * cal - ca( 0 ) * bal ) * denominator );
  return a + d;
}

Point2 Exact_adaptive_kernel::offcenter( Point2 const& a, Point2 const& b, Point2 const& c, double offconstant )
{
  Point2 ba = b - a;
  Point2 ca = c - a;
  Point2 bc = b - c;
  double abdist = distance_squared( a, b );
  double acdist = distance_squared( a, c );
  double bcdist = distance_squared( b, c );
  double denominator = 0.25 / signed_area( b, c, a );
  BOOST_ASSERT( denominator > 0.0 );
  double dx = ( ca(1) * abdist - ba(1) * acdist ) * denominator;
  double dy = ( ba(0) * acdist - ca(0) * abdist ) * denominator;
  double dxoff, dyoff;

  if ( ( abdist < acdist ) && ( abdist < bcdist ) )
  {
    dxoff = 0.5 * ba(0) - offconstant * ba(1);
    dyoff = 0.5 * ba(1) + offconstant * ba(0);

    if ( dxoff * dxoff + dyoff * dyoff < dx * dx + dy * dy )
    {
      dx = dxoff;
      dy = dyoff;
    }
  }
  else if ( acdist < bcdist )
  {
    dxoff = 0.5 * ca(0) + offconstant * ca(1);
    dyoff = 0.5 * ca(1) - offconstant * ca(0);

    if ( dxoff * dxoff + dyoff * dyoff < dx * dx + dy * dy )
    {
      dx = dxoff;
      dy = dyoff;
    }
  }
  else
  {
    dxoff = 0.5 * bc(0) - offconstant * bc(1);
    dyoff = 0.5 * bc(1) + offconstant * bc(0);

    if ( dxoff * dxoff + dyoff * dyoff < ( dx - ba(0) ) * ( dx - ba(0) ) + ( dy - ba(1) ) * ( dy - ba(1) ) )
    {
      dx = ba(0) + dxoff;
      dy = ba(1) + dyoff;
    }
  }

  return Point2( a(0) + dx, a(1) + dy );
}

double Exact_adaptive_kernel::signed_area( Point2 const& pa, Point2 const& pb, Point2 const& pc )
{
  return 0.5 * orient2d( pa.data(), pb.data(), pc.data() );
}

} // namespace umeshu
