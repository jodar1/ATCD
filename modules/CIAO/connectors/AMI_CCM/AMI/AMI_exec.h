// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler v1.7.1 ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from
// be/be_codegen.cpp:1217

#ifndef CIAO_AMI_EXEC_H_
#define CIAO_AMI_EXEC_H_


#include "AMIEC.h"
#include "AMI_exec_export.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/LocalObject.h"
#include "ace/Task.h"
#include "AMI_server.h"

#define AMI_CORBA_IMPLEMENTATION

namespace CIAO_Hello_AMI_AMI_Impl
{
#if !defined (AMI_CORBA_IMPLEMENTATION)
  /////////////////////////////
  //AMI thread implemenatation
  /////////////////////////////
  class ami_handler : public virtual ACE_Task_Base
  {
  public:
    ami_handler (
      ::CCM_AMI::Cookie ck,
      const  char * in_str,
      ::CCM_AMI::AMI_foo_ptr foo_receiver,
      ::CCM_AMI::AMI_foo_callback_ptr foo_callback);
    virtual int svc (void);
  private:
    long ck_;
    const char * in_str_;
    ::CCM_AMI::AMI_foo_ptr          foo_receiver_;
    ::CCM_AMI::AMI_foo_callback_ptr foo_callback_;
  };
#endif /* AMI_CORBA_IMPLEMENTATION */

  class  AMI_ami_foo_exec_i
    : public virtual ::CCM_AMI::CCM_AMI_ami_foo,
      public virtual ::CORBA::LocalObject
  {
  public:
#if !defined (AMI_CORBA_IMPLEMENTATION)
    AMI_ami_foo_exec_i (
      ::CCM_AMI::AMI_foo_ptr foo_receiver,
      ::CCM_AMI::AMI_foo_callback_ptr foo_callback);
#else
    AMI_ami_foo_exec_i (
      ::CCM_AMI::AMI_foo_callback_ptr foo_callback);
#endif /* AMI_CORBA_IMPLEMENTATION */

    virtual ~AMI_ami_foo_exec_i (void);
    
    // Operations and attributes from ::CCM_AMI::AMI_ami_foo
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46
    
    virtual ::CCM_AMI::Cookie
    sendc_asynch_foo (
      const char * in_str);
  private:
#if !defined (AMI_CORBA_IMPLEMENTATION)
    ::CCM_AMI::AMI_foo_var          foo_receiver_;
#endif /* AMI_CORBA_IMPLEMENTATION */
    ::CCM_AMI::AMI_foo_callback_var foo_callback_;
    long cookie_;

#if defined (AMI_CORBA_IMPLEMENTATION)
    CORBA::ORB_var orb_;
    INTERNAL_CCM_AMI::AMI_AMI_fooHandler_var the_handler_var_;
    INTERNAL_CCM_AMI::AMI_foo_var ami_foo_var_;
#endif /* AMI_CORBA_IMPLEMENTATION */

  };
  
  class  AMI_exec_i
    : public virtual AMI_Exec,
      public virtual ::CORBA::LocalObject
  {
  public:
    AMI_exec_i (void);
    virtual ~AMI_exec_i (void);
    
    // Supported operations and attributes.
    
    // Component attributes.
    
    // Port operations.
    
    virtual ::CCM_AMI::CCM_AMI_ami_foo_ptr
    get_perform_asynch_foo (void);
    
    // Operations from Components::SessionComponent.
    
    virtual void
    set_session_context (
      ::Components::SessionContext_ptr ctx);
    
    virtual void configuration_complete (void);
    
    virtual void ccm_activate (void);
    virtual void ccm_passivate (void);
    virtual void ccm_remove (void);
  
  private:
    ::Hello_AMI::CCM_AMI_Context_var context_;
  };
  
  extern "C" AMI_EXEC_Export ::Components::EnterpriseComponent_ptr
  create_Hello_AMI_AMI_Impl (void);
}

namespace CIAO_Hello_AMI_AMI_Impl
{
  class  AMIHome_exec_i
    : public virtual AMIHome_Exec,
      public virtual ::CORBA::LocalObject
  {
  public:
    AMIHome_exec_i (void);
    
    virtual ~AMIHome_exec_i (void);
    
    // All operations and attributes.
    
    // Factory operations.
    
    // Finder operations.
    
    // Implicit operations.
    
    virtual ::Components::EnterpriseComponent_ptr
    create (void);
  };
  
  extern "C" AMI_EXEC_Export ::Components::HomeExecutorBase_ptr
  create_Hello_AMI_AMIHome_Impl (void);
}

#endif /* ifndef */

