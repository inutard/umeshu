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

#ifndef UMESHU_TRIANGULATION_ITEMS_H
#define UMESHU_TRIANGULATION_ITEMS_H

#include "HDS/HDS_node_base.h"
#include "HDS/HDS_halfedge_base.h"
#include "HDS/HDS_edge_base.h"
#include "HDS/HDS_face_base.h"
#include "Point2.h"
#include "Orientation.h"

namespace umeshu {

template <typename Kernel, typename HDS>
class Triangulation_node_base : public hds::HDS_node_base<HDS>
{

  typedef hds::HDS_node_base<HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

  Triangulation_node_base()
    : Base()
    , position_()
  {}

  explicit Triangulation_node_base( Point2 const& p )
    : Base()
    , position_( p )
  {}

  void set_position( Point2 const& p )
  {
    position_ = p;
  }

  Point2 const& position() const
  {
    return position_;
  }

  unsigned degree() const
  {
    unsigned d = 0;

    if ( ! this->is_isolated() )
    {
      Halfedge_handle he_start = this->halfedge();
      Halfedge_handle he_iter = he_start;

      do
      {
        ++d;
        he_iter = he_iter->pair()->next();
      }
      while ( he_iter != he_start );
    }

    return d;
  }

  Halfedge_handle boundary_halfedge() const
  {
    if ( this->is_isolated() )
    {
      return Halfedge_handle();
    }

    Halfedge_handle bhe_start = this->halfedge();
    Halfedge_handle bhe_iter = bhe_start;

    do
    {
      if ( bhe_iter->is_boundary() )
      {
        return bhe_iter;
      }

      bhe_iter = bhe_iter->pair()->next();
    }
    while ( bhe_iter != bhe_start );

    return Halfedge_handle();
  }

  bool is_boundary() const
  {
    return boundary_halfedge() != Halfedge_handle();
  }

private:

  Point2 position_;

};


template <typename Kernel, typename HDS>
class Triangulation_node_base_with_id : public Triangulation_node_base<Kernel, HDS>
                                      , public Identifiable
{

  typedef Triangulation_node_base<Kernel, HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

};


template <typename Kernel, typename HDS>
class Triangulation_halfedge_base : public hds::HDS_halfedge_base<HDS>
{

  typedef hds::HDS_halfedge_base<HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

  bool is_boundary() const
  {
    return this->face() == Face_handle();
  }

};


template <typename Kernel, typename HDS>
class Triangulation_halfedge_base_with_id : public Triangulation_halfedge_base<Kernel, HDS>
                                          , public Identifiable
{

  typedef Triangulation_halfedge_base<Kernel, HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

};


template <typename Kernel, typename HDS>
class Triangulation_edge_base : public hds::HDS_edge_base<HDS>
{

  typedef hds::HDS_edge_base<HDS>     Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

  Triangulation_edge_base( Halfedge_handle g, Halfedge_handle h )
    : Base( g, h )
  {}

  bool is_boundary() const
  {
    return this->he1()->is_boundary() || this->he2()->is_boundary();
  }

  void vertices( Point2& p1, Point2& p2 ) const
  {
    p1 = this->he1()->origin()->position();
    p2 = this->he2()->origin()->position();
  }

  double length() const
  {
    Point2 p1, p2;
    vertices( p1, p2 );
    return Kernel::distance( p1, p2 );
  }

  bool is_diagonal_of_convex_quadrilateral() const
  {
    if ( this->is_boundary() )
    {
      return false;
    }

    Point2 p1 = this->he1()->origin()->position();
    Point2 p2 = this->he2()->prev()->origin()->position();
    Point2 p3 = this->he2()->origin()->position();
    Point2 p4 = this->he1()->prev()->origin()->position();

    if ( Kernel::oriented_side( p1, p3, p2 ) != ON_NEGATIVE_SIDE ||
         Kernel::oriented_side( p1, p3, p4 ) != ON_POSITIVE_SIDE )
    {
      return false;
    }
    // if ( Kernel::oriented_side( p1, p2, p3 ) != ON_POSITIVE_SIDE ||
    //      Kernel::oriented_side( p2, p3, p4 ) != ON_POSITIVE_SIDE ||
    //      Kernel::oriented_side( p3, p4, p1 ) != ON_POSITIVE_SIDE ||
    //      Kernel::oriented_side( p4, p1, p2 ) != ON_POSITIVE_SIDE )
    // {
    //   return false;
    // }

    return true;
  }

  void flip()
  {
    BOOST_ASSERT( is_diagonal_of_convex_quadrilateral() );
    Halfedge_handle h1 = this->he1();
    Halfedge_handle h2 = this->he2();
    Face_handle f1 = h1->face();
    Face_handle f2 = h2->face();
    Node_handle n1 = h1->origin();
    Node_handle n2 = h2->origin();
    Halfedge_handle h3 = h1->next();
    Halfedge_handle h4 = h1->prev();
    Halfedge_handle h5 = h2->next();
    Halfedge_handle h6 = h2->prev();
    Node_handle n3 = h6->origin();
    Node_handle n4 = h4->origin();

    f1->set_halfedge( h1 );
    f2->set_halfedge( h2 );

    h1->set_face( f1 );
    h4->set_face( f1 );
    h5->set_face( f1 );
    h2->set_face( f2 );
    h6->set_face( f2 );
    h3->set_face( f2 );

    if ( n1->halfedge() == h1 )
    {
      n1->set_halfedge( h5 );
    }

    if ( n2->halfedge() == h2 )
    {
      n2->set_halfedge( h3 );
    }

    h1->set_origin( n3 );
    h2->set_origin( n4 );

    h1->set_next( h4 );
    h4->set_next( h5 );
    h5->set_next( h1 );
    h1->set_prev( h5 );
    h5->set_prev( h4 );
    h4->set_prev( h1 );

    h2->set_next( h6 );
    h6->set_next( h3 );
    h3->set_next( h2 );
    h2->set_prev( h3 );
    h3->set_prev( h6 );
    h6->set_prev( h2 );
  }
};


template <typename Kernel, typename HDS>
class Triangulation_edge_base_with_id : public Triangulation_edge_base<Kernel, HDS>
                                      , public Identifiable
{

  typedef Triangulation_edge_base<Kernel, HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

};


template <typename Kernel, typename HDS>
class Triangulation_face_base : public hds::HDS_face_base<HDS>
{

  typedef hds::HDS_face_base<HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

  bool is_triangle() const
  {
    return this->halfedge()->prev() == this->halfedge()->next()->next();
  }

  void nodes( Node_handle& n1, Node_handle& n2, Node_handle& n3 ) const
  {
    BOOST_ASSERT( is_triangle() );
    n1 = this->halfedge()->origin();
    n2 = this->halfedge()->pair()->origin();
    n3 = this->halfedge()->prev()->origin();
  }

  void vertices( Point2& p1, Point2& p2, Point2& p3 ) const
  {
    BOOST_ASSERT( is_triangle() );
    Node_handle n1, n2, n3;
    nodes( n1, n2, n3 );
    p1 = n1->position();
    p2 = n2->position();
    p3 = n3->position();
  }

};


template <typename Kernel, typename HDS>
class Triangulation_face_base_with_id : public Triangulation_face_base<Kernel, HDS>
                                      , public Identifiable
{

  typedef Triangulation_face_base<Kernel, HDS> Base;

public:

  typedef typename Base::Node_handle     Node_handle;
  typedef typename Base::Halfedge_handle Halfedge_handle;
  typedef typename Base::Edge_handle     Edge_handle;
  typedef typename Base::Face_handle     Face_handle;

};


struct Triangulation_items
{

  typedef boost::false_type Supports_intrusive_list;
  typedef boost::false_type Supports_id;

  template <typename Kernel, typename HDS>
  struct Node_wrapper
  {
    typedef Triangulation_node_base<Kernel, HDS> Node;
  };

  template <typename Kernel, typename HDS>
  struct Halfedge_wrapper
  {
    typedef Triangulation_halfedge_base<Kernel, HDS> Halfedge;
  };

  template <typename Kernel, typename HDS>
  struct Edge_wrapper
  {
    typedef Triangulation_edge_base<Kernel, HDS> Edge;
  };

  template <typename Kernel, typename HDS>
  struct Face_wrapper
  {
    typedef Triangulation_face_base<Kernel, HDS> Face;
  };

};


struct Triangulation_items_with_id
{

  typedef boost::false_type Supports_intrusive_list;
  typedef boost::true_type  Supports_id;

  template <typename Kernel, typename HDS>
  struct Node_wrapper
  {
    typedef Triangulation_node_base_with_id<Kernel, HDS> Node;
  };

  template <typename Kernel, typename HDS>
  struct Halfedge_wrapper
  {
    typedef Triangulation_halfedge_base_with_id<Kernel, HDS> Halfedge;
  };

  template <typename Kernel, typename HDS>
  struct Edge_wrapper
  {
    typedef Triangulation_edge_base_with_id<Kernel, HDS> Edge;
  };

  template <typename Kernel, typename HDS>
  struct Face_wrapper
  {
    typedef Triangulation_face_base_with_id<Kernel, HDS> Face;
  };

};

} // namespace umeshu

#endif // UMESHU_TRIANGULATION_ITEMS_H
