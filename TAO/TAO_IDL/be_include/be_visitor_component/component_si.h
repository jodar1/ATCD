//
// $Id$
//
/* -*- c++ -*- */
// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    component_si.h
//
// = DESCRIPTION
//    Concrete visitor for the Component node.
//    This provides for code generation in the server inline
//
// = AUTHOR
//    Jeff Parsons
//
// ============================================================================

#ifndef _BE_COMPONENT_COMPONENT_SI_H_
#define _BE_COMPONENT_COMPONENT_SI_H_

class be_visitor_component_si : public be_visitor_component
{
  //
  // = TITLE
  //   be_visitor_component_sh
  //
  // = DESCRIPTION
  //   This is a concrete visitor to generate the server header for component.
  //
  //
public:
  be_visitor_component_si (be_visitor_context *ctx);
  // constructor
  
  ~be_visitor_component_si (void);
  // destructor
  
  virtual int visit_component (be_component *node);
  // set the right context and make a visitor
};

#endif /* _BE_COMPONENT_COMPONENT_SI_H_ */