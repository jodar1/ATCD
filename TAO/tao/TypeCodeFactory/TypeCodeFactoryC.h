// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
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
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from
// be/be_codegen.cpp:151

#ifndef _TAO_IDL_ORIG_TYPECODEFACTORYC_H_
#define _TAO_IDL_ORIG_TYPECODEFACTORYC_H_

#include "ace/pre.h"

#ifndef TAO_TYPECODEFACTORY_SAFE_INCLUDE
#error "You should not include TypeCodeFactoryC.h directly, use TCF_Loader.h"
#endif /* !TAO_TYPECODEFACTORY_SAFE_INCLUDE */

#include "ace/pre.h"
#include "typecodefactory_export.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/Object.h"

#if defined (TAO_EXPORT_MACRO)
#undef TAO_EXPORT_MACRO
#endif
#define TAO_EXPORT_MACRO TAO_TypeCodeFactory_Export

#if defined (TAO_EXPORT_NESTED_CLASSES)
#  if defined (TAO_EXPORT_NESTED_MACRO)
#    undef TAO_EXPORT_NESTED_MACRO
#  endif /* defined (TAO_EXPORT_NESTED_MACRO) */
#  define TAO_EXPORT_NESTED_MACRO TAO_TypeCodeFactory_Export
#endif /* TAO_EXPORT_NESTED_CLASSES */

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#endif /* _MSC_VER >= 1200 */
#pragma warning(disable:4250)
#endif /* _MSC_VER */

#if defined (__BORLANDC__)
#pragma option push -w-rvl -w-rch -w-ccc -w-inl
#endif /* __BORLANDC__ */

// TAO_IDL - Generated from
// be/be_visitor_module/module_ch.cpp:48

TAO_NAMESPACE  CORBA
{

  // TAO_IDL - Generated from
  // be/be_visitor_typedef/typedef_ch.cpp:368

  typedef char * Identifier;
  typedef CORBA::String_var Identifier_var;
  typedef CORBA::String_out Identifier_out;

  // TAO_IDL - Generated from
  // be/be_visitor_typecode/typecode_decl.cpp:44

  TAO_NAMESPACE_STORAGE_CLASS ::CORBA::TypeCode_ptr _tc_Identifier;

  // TAO_IDL - Generated from
  // be/be_visitor_typedef/typedef_ch.cpp:368

  typedef char * RepositoryId;
  typedef CORBA::String_var RepositoryId_var;
  typedef CORBA::String_out RepositoryId_out;

  // TAO_IDL - Generated from
  // be/be_visitor_typecode/typecode_decl.cpp:44

  TAO_NAMESPACE_STORAGE_CLASS ::CORBA::TypeCode_ptr _tc_RepositoryId;

  // TAO_IDL - Generated from
  // be/be_visitor_typedef/typedef_ch.cpp:334

  typedef CORBA::Short ValueModifier;
  typedef CORBA::Short_out ValueModifier_out;

  // TAO_IDL - Generated from
  // be/be_visitor_typecode/typecode_decl.cpp:44

  TAO_NAMESPACE_STORAGE_CLASS ::CORBA::TypeCode_ptr _tc_ValueModifier;

  // TAO_IDL - Generated from
  // be/be_visitor_interface/interface_ch.cpp:52

#if !defined (_CORBA_TYPECODEFACTORY___PTR_CH_)
#define _CORBA_TYPECODEFACTORY___PTR_CH_

  class TypeCodeFactory;
  typedef TypeCodeFactory *TypeCodeFactory_ptr;

#endif /* end #if !defined */

#if !defined (_CORBA_TYPECODEFACTORY___VAR_CH_)
#define _CORBA_TYPECODEFACTORY___VAR_CH_

  // TAO_IDL - Generated from
  // be/be_interface.cpp:565

  class TAO_TypeCodeFactory_Export TypeCodeFactory_var : private TAO_Base_var
  {
  public:
    TypeCodeFactory_var (void); // default constructor
    TypeCodeFactory_var (TypeCodeFactory_ptr p) : ptr_ (p) {}
    TypeCodeFactory_var (const TypeCodeFactory_var &); // copy constructor
    ~TypeCodeFactory_var (void); // destructor

    TypeCodeFactory_var &operator= (TypeCodeFactory_ptr);
    TypeCodeFactory_var &operator= (const TypeCodeFactory_var &);
    TypeCodeFactory_ptr operator-> (void) const;

    operator const TypeCodeFactory_ptr &() const;
    operator TypeCodeFactory_ptr &();
    // in, inout, out, _retn
    TypeCodeFactory_ptr in (void) const;
    TypeCodeFactory_ptr &inout (void);
    TypeCodeFactory_ptr &out (void);
    TypeCodeFactory_ptr _retn (void);
    TypeCodeFactory_ptr ptr (void) const;

    // Hooks used by template sequence and object manager classes
    // for non-defined forward declared interfaces.
    static TypeCodeFactory_ptr tao_duplicate (TypeCodeFactory_ptr);
    static void tao_release (TypeCodeFactory_ptr);
    static TypeCodeFactory_ptr tao_nil (void);
    static TypeCodeFactory_ptr tao_narrow (
        CORBA::Object *
        ACE_ENV_ARG_DECL
      );
    static CORBA::Object * tao_upcast (void *);

  private:
    TypeCodeFactory_ptr ptr_;
    // Unimplemented - prevents widening assignment.
    TypeCodeFactory_var (const TAO_Base_var & rhs);
    TypeCodeFactory_var & operator= (const TAO_Base_var & rhs);
  };

#endif /* end #if !defined */

#if !defined (_CORBA_TYPECODEFACTORY___OUT_CH_)
#define _CORBA_TYPECODEFACTORY___OUT_CH_

  // TAO_IDL - Generated from
  // be/be_interface.cpp:946

  class TAO_TypeCodeFactory_Export TypeCodeFactory_out
  {
  public:
    TypeCodeFactory_out (TypeCodeFactory_ptr &);
    TypeCodeFactory_out (TypeCodeFactory_var &);
    TypeCodeFactory_out (const TypeCodeFactory_out &);
    TypeCodeFactory_out &operator= (const TypeCodeFactory_out &);
    TypeCodeFactory_out &operator= (const TypeCodeFactory_var &);
    TypeCodeFactory_out &operator= (TypeCodeFactory_ptr);
    operator TypeCodeFactory_ptr &();
    TypeCodeFactory_ptr &ptr (void);
    TypeCodeFactory_ptr operator-> (void);

  private:
    TypeCodeFactory_ptr &ptr_;
  };

#endif /* end #if !defined */

  // TAO_IDL - Generated from
  // be/be_visitor_interface/interface_ch.cpp:110

#if !defined (_CORBA_TYPECODEFACTORY_CH_)
#define _CORBA_TYPECODEFACTORY_CH_

  class TAO_TypeCodeFactory_Export TypeCodeFactory
    : public virtual CORBA::Object
  {
  public:
    typedef TypeCodeFactory_ptr _ptr_type;
    typedef TypeCodeFactory_var _var_type;
    static int _tao_class_id;

    // The static operations.
    static TypeCodeFactory_ptr _duplicate (TypeCodeFactory_ptr obj);

    static TypeCodeFactory_ptr _narrow (
        CORBA::Object_ptr obj
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );

    static TypeCodeFactory_ptr _unchecked_narrow (
        CORBA::Object_ptr obj
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );

    static TypeCodeFactory_ptr _nil (void)
      {
        return (TypeCodeFactory_ptr)0;
      }

    static void _tao_any_destructor (void *);

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_struct_tc (
        const char * id,
        const char * name,
        const CORBA::StructMemberSeq & members
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_union_tc (
        const char * id,
        const char * name,
        CORBA::TypeCode_ptr discriminator_type,
        const CORBA::UnionMemberSeq & members
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_enum_tc (
        const char * id,
        const char * name,
        const CORBA::EnumMemberSeq & members
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_alias_tc (
        const char * id,
        const char * name,
        CORBA::TypeCode_ptr original_type
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_exception_tc (
        const char * id,
        const char * name,
        const CORBA::StructMemberSeq & members
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_interface_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_string_tc (
        CORBA::ULong bound
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_wstring_tc (
        CORBA::ULong bound
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_fixed_tc (
        CORBA::UShort digits,
        CORBA::UShort scale
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_sequence_tc (
        CORBA::ULong bound,
        CORBA::TypeCode_ptr element_type
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_array_tc (
        CORBA::ULong length,
        CORBA::TypeCode_ptr element_type
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_value_tc (
        const char * id,
        const char * name,
        CORBA::ValueModifier type_modifier,
        CORBA::TypeCode_ptr concrete_base,
        const CORBA::ValueMemberSeq & members
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_value_box_tc (
        const char * id,
        const char * name,
        CORBA::TypeCode_ptr boxed_type
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_native_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_recursive_tc (
        const char * id
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_abstract_interface_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_local_interface_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_component_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_ch.cpp:46

    virtual CORBA::TypeCode_ptr create_home_tc (
        const char * id,
        const char * name
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      )
      ACE_THROW_SPEC ((
        CORBA::SystemException
      )) = 0;

    // TAO_IDL - Generated from
    // be/be_visitor_interface/interface_ch.cpp:267

    virtual void *_tao_QueryInterface (ptr_arith_t type);

    virtual const char* _interface_repository_id (void) const;

  protected:
    TypeCodeFactory (void);
    virtual ~TypeCodeFactory (void);

  private:
    TypeCodeFactory (const TypeCodeFactory &);
    void operator= (const TypeCodeFactory &);
  };

#endif /* end #if !defined */

  // TAO_IDL - Generated from
  // be/be_visitor_typecode/typecode_decl.cpp:44

  TAO_NAMESPACE_STORAGE_CLASS ::CORBA::TypeCode_ptr _tc_TypeCodeFactory;

// TAO_IDL - Generated from
// be/be_visitor_module/module_ch.cpp:67

}
TAO_NAMESPACE_CLOSE // module CORBA

// TAO_IDL - Generated from
// be/be_visitor_interface/any_op_ch.cpp:52

TAO_TypeCodeFactory_Export void operator<<= (CORBA::Any &, CORBA::TypeCodeFactory_ptr); // copying
TAO_TypeCodeFactory_Export void operator<<= (CORBA::Any &, CORBA::TypeCodeFactory_ptr *); // non-copying
TAO_TypeCodeFactory_Export CORBA::Boolean operator>>= (const CORBA::Any &, CORBA::TypeCodeFactory_ptr &);

// TAO_IDL - Generated from
// be/be_visitor_root/cdr_op.cpp:48

#ifndef __ACE_INLINE__

// TAO_IDL - Generated from
// be/be_visitor_root/cdr_op.cpp:64

#endif /* __ACE_INLINE__ */

// TAO_IDL - Generated from
// be/be_codegen.cpp:1016

#if defined (__ACE_INLINE__)
#include "TypeCodeFactoryC.inl"
#endif /* defined INLINE */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(pop)
#endif /* _MSC_VER */

#if defined (__BORLANDC__)
#pragma option pop
#endif /* __BORLANDC__ */

#include "ace/post.h"
#endif /* ifndef */
