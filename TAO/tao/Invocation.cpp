// $Id$

#include "tao/corba.h"

#include "tao/Timeprobe.h"

#if !defined (__ACE_INLINE__)
# include "tao/Invocation.i"
#endif /* ! __ACE_INLINE__ */

ACE_RCSID(tao, Invocation, "$Id$")

#if defined (ACE_ENABLE_TIMEPROBES)

  static const char *TAO_Invocation_Timeprobe_Description[] =
{
  "GIOP_Invocation::start - enter",
  "GIOP_Invocation::start - leave",
  "GIOP_Invocation::start - connect",
  "GIOP_Invocation::start - start_msg",
  "GIOP_Invocation::start - request_hdr"
};

enum
{
  TAO_GIOP_INVOCATION_START_ENTER = 1000,
  TAO_GIOP_INVOCATION_START_LEAVE,
  TAO_GIOP_INVOCATION_START_CONNECT,
  TAO_GIOP_INVOCATION_START_START_MSG,
  TAO_GIOP_INVOCATION_START_REQUEST_HDR
};


// Setup Timeprobes
ACE_TIMEPROBE_EVENT_DESCRIPTIONS (TAO_Invocation_Timeprobe_Description,
                                  TAO_GIOP_INVOCATION_START_ENTER);

#endif /* ACE_ENABLE_TIMEPROBES */

// Normal invocations don't involve any heap al; messages are
// constructed into stack-based buffers and are read into those
// buffers too.  Larger buffers are heap-allocated as needed.
//
// The constraint on request IDs is that no two requests from the same
// client with the same ID are outstanding at the same time.  In
// single threaded environments, this is met by any number whatever.
// When multiple threads are used, we eliminate the need for any
// locked state by using the thread ID as the request ID, since any
// given thread has at most one request outstanding at a time.
//
// NOTE: this means that if "deferred synchronous" calls get
// supported, it's done by creating a thread internally to make the
// call.  That is less disruptive (and error prone) in general than
// restructuring an ORB core in terms of asynchrony.

TAO_GIOP_Invocation::TAO_GIOP_Invocation (IIOP_Object *data,
                                          const char *operation,
                                          TAO_ORB_Core* orb_core)
  : data_ (data),
    opname_ (operation),
    my_request_id_ (0),
    out_stream_ (buffer, sizeof buffer, /* CDR::DEFAULT_BUFSIZE */
                 TAO_ENCAP_BYTE_ORDER,
                 TAO_Marshal::DEFAULT_MARSHAL_FACTORY,
                 orb_core->output_cdr_buffer_allocator (),
                 orb_core->output_cdr_dblock_allocator ()),
    orb_core_ (orb_core)
{
  // @@ TODO The comments here are scary, can someone please give me a
  // warm fuzzy feeling about this (coryan).

  // The assumption that thread ids are ints is false and horribly
  // implementation-dependent, so this code just sucks.  But, at least
  // it will compile on multiple platforms through the magic of ACE
  // :-/

  //assert (sizeof (CORBA::ULong) == sizeof (ACE_thread_t));
  ACE_thread_t me = ACE_OS::thr_self ();

  // Copy in only as many bytes are valid, or only as many as we have
  // room for, whichever is less.  -------> What a friggin' HACK!?!?!
  ACE_OS::memcpy (&this->my_request_id_,
                  &me,
                  ACE_MIN (sizeof (me), sizeof (this->my_request_id_)));
}

TAO_GIOP_Invocation::~TAO_GIOP_Invocation (void)
{
  if (this->data_->handler () != 0)
    this->data_->handler ()->idle ();
}

// The public API involves creating an invocation, starting it, filling
// in request parameters, actually performing the invocation, getting
// response parameters, and then cleaning up.  Sometimes they must be
// restarted (e.g. request forwarding).  This is the start/restart entry.

void
TAO_GIOP_Invocation::start (CORBA::Boolean is_roundtrip,
                            TAO_GIOP::Message_Type message_type,
                            CORBA::Environment &env)
{
  ACE_FUNCTION_TIMEPROBE (TAO_GIOP_INVOCATION_START_ENTER);

  // First try to bind to the appropriate address.  We do that here
  // since we may get forwarded to a different objref in the course of
  // any given call, with new start () call each time.  It's not
  // cached in the objref data since the connections change
  // asynchronously from objref invocations and this simplifies
  // connection management.
  // We also need to bind *before* marshalling, because different
  // Profiles have different ObjectKeys, thus a change of Profile can
  // result in different alignment for the buffer.
  //
  // THREADING NOTE: this connection is reserved to this call.  Also,
  // starting at this point in the call, new forwarding information
  // will not be used until/unless the call is reissued.  Correctness
  // is not affected, the call will just be forwarded later than it
  // might be in a more complex implementation.

  // @@ assert is evil, it crashes the program, changed to an
  // exception (coryan)
  // assert (this->data_ != 0);

  if (this->data_ == 0)
    TAO_THROW_ENV_RETURN_VOID (CORBA::MARSHAL (CORBA::COMPLETED_NO), env);

  // Get a pointer to the connector, which might be in thread-specific
  // storage, depending on the concurrency model.
  TAO_CONNECTOR *con = this->orb_core_->connector ();

  // Determine the object key and the address to which we'll need a
  // connection.
  const TAO_opaque *key;
  ACE_INET_Addr *server_addr_p = 0;

  {
    ACE_MT (ACE_GUARD (ACE_Lock, guard, data_->get_fwd_profile_lock ()));

    if (data_->get_fwd_profile_i () != 0)
      {
        key = &data_->get_fwd_profile_i ()->object_key;
        server_addr_p = &data_->get_fwd_profile_i ()->object_addr ();
      }
    else
      {
        key = &data_->profile.object_key;
        server_addr_p = &data_->profile.object_addr ();
      }
  }

  if (server_addr_p == 0)
    TAO_THROW_ENV_RETURN_VOID (CORBA::INTERNAL (CORBA::COMPLETED_NO), env);

  // Establish the connection and get back a
  // <Client_Connection_Handler>.
#if defined (TAO_ARL_USES_SAME_CONNECTOR_PORT)
  if (this->orb_core_->arl_same_port_connect ())
    {
      ACE_INET_Addr local_addr (this->orb_core_->orb_params ()->addr ());
      local_addr.set_port_number (server_addr_p->get_port_number ());

      // Set the local port number to use.

      if (con->connect (this->data_->handler (),
                        *server_addr_p,
                        0,
                        local_addr,
                        1) == -1)
        {
          // Give users a clue to the problem.
          ACE_DEBUG ((LM_ERROR, "(%P|%t) %s:%u, connection to "
                      "%s (%s):%hu failed (%p) (using local port #: %d)\n",
                      __FILE__,
                      __LINE__,
                      server_addr_p->get_host_name (),
                      server_addr_p->get_host_addr (),
                      server_addr_p->get_port_number (),
                      local_addr.get_port_number (),
                      "errno"));

          TAO_THROW_ENV_RETURN_VOID (CORBA::TRANSIENT (CORBA::COMPLETED_NO), env);
        }
    }
  else
#endif /* TAO_ARL_USES_SAME_CONNECTOR_PORT */
    if (con->connect (this->data_->handler (),
                      *server_addr_p) == -1)
      {
        // Give users a clue to the problem.
        if (TAO_orbdebug)
          ACE_DEBUG ((LM_ERROR, "(%P|%t) %s:%u, connection to "
                      "%s (%s):%hu failed (%p)\n",
                      __FILE__,
                      __LINE__,
                      server_addr_p->get_host_name (),
                      server_addr_p->get_host_addr (),
                      server_addr_p->get_port_number (),
                      "errno"));

        TAO_THROW_ENV_RETURN_VOID (CORBA::TRANSIENT (CORBA::COMPLETED_NO), env);
      }

  ACE_TIMEPROBE (TAO_GIOP_INVOCATION_START_CONNECT);

  // Use the TAO_SOCK_Stream from the Client_Connection_Handler for
  // communication inplace of the endpoint used below.

  // POLICY DECISION: If the client expects most agents to forward,
  // then it could try to make sure that it's been forwarded at least
  // once by eliciting it with a LocateRequest message. (Further
  // hinting in the IIOP::ProfileData could help!)
  //
  // That scenario does not match an "Inter" ORB Protocol well, since
  // bridges chain calls rather than forwarding them.  It does match
  // some kinds of "Intra" ORB scenarios well, with many agents that
  // spawn new processes talking to their clients across the net.
  //
  // At this time, the policy noted above is followed in the sense
  // that this software does NOT expect most agents to forward, so it
  // doesn't bother to probe.  Correctness is not affected; this is
  // only a quality-of-service policy.  It affects mostly performance,
  // but the "best efforts" semantics for "oneway" messages would also
  // be impacted in that some (by definition, buggy!) code which used
  // only "oneway" messages might not work at all.

  // Build the outgoing message, starting with generic GIOP header.

  CORBA::Boolean bt =
    TAO_GIOP::start_message (message_type,
                             this->out_stream_,
                             this->orb_core_);

  if (bt != 1)
    TAO_THROW_ENV_RETURN_VOID (CORBA::MARSHAL (CORBA::COMPLETED_NO), env);

  ACE_TIMEPROBE (TAO_GIOP_INVOCATION_START_START_MSG);

  // Then fill in the rest of the RequestHeader
  //
  // The first element of header is service context list;
  // transactional context would be acquired here using the
  // transaction service APIs.  Other kinds of context are as yet
  // undefined.
  //
  // Last element of request header is the principal; no portable way
  // to get it, we just pass empty principal (convention: indicates
  // "anybody").  Steps upward in security include passing an
  // unverified user ID, and then verifying the message (i.e. a dummy
  // service context entry is set up to hold a digital signature for
  // this message, then patched shortly before it's sent).
  static CORBA::Principal_ptr principal = 0;

  // This static is only used to write into the CDR stream, once we
  // have real service context (needed for the messaging spec) this
  // will have to be a parameter.
  static TAO_GIOP_ServiceContextList svc_ctx;

  switch (message_type)
    {
    case TAO_GIOP::Request:

      this->write_request_header (svc_ctx,
                                  this->my_request_id_,
                                  is_roundtrip,
                                  key,
                                  this->opname_,
                                  principal);
      break;

    case TAO_GIOP::LocateRequest:
      this->out_stream_ << this->my_request_id_;
      this->out_stream_ << *key;
      break;

    default:
      TAO_THROW_ENV_RETURN_VOID (CORBA::INTERNAL (CORBA::COMPLETED_NO), env);
    }
  if (!this->out_stream_.good_bit ())
    TAO_THROW_ENV_RETURN_VOID (CORBA::MARSHAL (CORBA::COMPLETED_NO), env);

  ACE_TIMEPROBE (TAO_GIOP_INVOCATION_START_REQUEST_HDR);
}

CORBA::Boolean
TAO_GIOP_Invocation::write_request_header_std
     (const TAO_GIOP_ServiceContextList& svc_ctx,
      CORBA::ULong request_id,
      CORBA::Boolean is_roundtrip,
      const TAO_opaque* key,
      const char* opname,
      CORBA::Principal_ptr principal)
{
  this->out_stream_ << svc_ctx;
  this->out_stream_ << request_id;
  this->out_stream_ << CORBA::Any::from_boolean (is_roundtrip);
  this->out_stream_ << *key;
  this->out_stream_ << opname;
  this->out_stream_ << principal;
  return 1;
}

CORBA::Boolean
TAO_GIOP_Invocation::write_request_header_lite
     (const TAO_GIOP_ServiceContextList&,
      CORBA::ULong request_id,
      CORBA::Boolean is_roundtrip,
      const TAO_opaque* key,
      const char* opname,
      CORBA::Principal_ptr)
{
  this->out_stream_ << request_id;
  this->out_stream_ << CORBA::Any::from_boolean (is_roundtrip);
  this->out_stream_ << *key;
  this->out_stream_ << opname;
  return 1;
}

CORBA::Boolean
TAO_GIOP_Invocation::write_request_header
     (const TAO_GIOP_ServiceContextList& svc_ctx,
      CORBA::ULong request_id,
      CORBA::Boolean is_roundtrip,
      const TAO_opaque* key,
      const char* opname,
      CORBA::Principal_ptr principal)
{
  if (this->orb_core_->orb_params ()->use_IIOP_lite_protocol ())
    return this->write_request_header_lite (svc_ctx,
                                            request_id,
                                            is_roundtrip,
                                            key,
                                            opname,
                                            principal);
  else
    return this->write_request_header_std (svc_ctx,
                                           request_id,
                                           is_roundtrip,
                                           key,
                                           opname,
                                           principal);
}


// Send request, block until any reply comes back, and unmarshal reply
// parameters as appropriate.

TAO_GIOP_ReplyStatusType
TAO_GIOP_Invocation::invoke (CORBA::Boolean is_roundtrip,
                             CORBA::Environment &env)
{
  // Send Request, return on error or if we're done

  if (this->data_->handler ()->send_request (this->orb_core_,
                                             this->out_stream_,
                                             is_roundtrip) == -1)
    {
      // send_request () closed the connection; we just set the
      // handler to 0 here.
      this->data_->reset_handler ();

      //
      // @@ highly desirable to know whether we wrote _any_ data; if
      // we wrote none, then there's no chance the call completed and
      // applications don't have to deal with those nasty
      // indeterminate states where they can't immediatly tell if
      // what's safe to do.
      //
      // @@ also, there might have been a GIOP::CloseConnection
      // message in the input queue.  If so, this request should be
      // treated as a (full) "rebind" case.  Can't do that from this
      // point in the code however!  Some minor restructuring needs to
      // happen.
      //
      TAO_THROW_ENV_RETURN (CORBA::TRANSIENT (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }
  return TAO_GIOP_NO_EXCEPTION;
}

// ****************************************************************

TAO_GIOP_ReplyStatusType
TAO_GIOP_Invocation::close_connection (void)
{
  // Special case of forwarding -- server was closing the
  // connection, which just indicates resource constraints, not an
  // error.  The client is effectively "forwarded" to the same
  // server!
  //
  // However, we must reinitialize the forwarding chain, since the
  // resource being reclaimed might also have been the process,
  // not just the connection.  Without reinitializing, we'd give
  // false error reports to applications.
  {
    ACE_MT (ACE_GUARD_RETURN (ACE_Lock,
                              guard,
                              data_->get_fwd_profile_lock (),
                              TAO_GIOP_SYSTEM_EXCEPTION));

    IIOP::Profile *old = data_->set_fwd_profile (0);
    delete old;
    // sets the forwarding profile to 0 and deletes the old one;

    data_->reset_first_locate_request ();
    // resets the flag of the first call locate request to true
  }

  this->data_->handler ()->handle_close ();
  this->data_->reset_handler ();
  return TAO_GIOP_LOCATION_FORWARD;
}


// Handle the GIOP Reply with status = LOCATION_FORWARD
// Replace the IIOP Profile. The call is then automatically
// reinvoked by the IIOP_Object::do_static_call method.

TAO_GIOP_ReplyStatusType
TAO_GIOP_Invocation::location_forward (TAO_InputCDR &inp_stream,
                                       CORBA::Environment &env)
{
  // It can be assumed that the GIOP header and the reply header
  // are already handled. Further it can be assumed that the
  // reply body contains an object reference to the new object.
  // This object pointer will be now extracted.

  CORBA::Object_ptr object_ptr = 0;

  TAO_TRY_VAR (env)
    {
      inp_stream.decode (CORBA::_tc_Object,
                         &(object_ptr),
                         0,
                         env);
      TAO_CHECK_ENV;
    }
  TAO_CATCH (CORBA_SystemException, ex)
    {
      // Handle the exception for this level here and throw it out again.
      dexc (env, "invoke, location forward (decode)");
      this->data_->handler ()->handle_close ();
      TAO_RETHROW_SAME_ENV_RETURN (TAO_GIOP_SYSTEM_EXCEPTION);
    }
  TAO_ENDTRY;

  // The object pointer has to be changed to a IIOP_Object pointer
  // in order to extract the profile.

  IIOP_Object *iiopobj =
    ACE_dynamic_cast (IIOP_Object*, object_ptr->_stubobj ());

  if (iiopobj == 0)
    {
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::UNKNOWN (CORBA::COMPLETED_NO), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  // Make a copy of the IIOP profile in the forwarded objref,
  // reusing memory where practical.  Then delete the forwarded
  // objref, retaining only its profile.
  //
  // @@ add and use a "forward count", to prevent loss of data
  // in forwarding chains during concurrent calls -- only a
  // forward that's a response to the current fwd_profile should
  // be recorded here. (This is just an optimization, and is not
  // related to correctness.)

  // the copy method on IIOP::Profile will be used to copy the content
  data_->set_fwd_profile (&iiopobj->profile);
  // store the new profile in the forwarding profile
  // note: this has to be and is thread safe

  // The object is no longer needed, because we have now the IIOP_Object
  // @@ Is this exception safe?
  CORBA::release (object_ptr);

  env.clear ();

  // We may not need to do this since TAO_GIOP_Invocations
  // get created on a per-call basis. For now we'll play it safe.

  return TAO_GIOP_LOCATION_FORWARD;
}

// ****************************************************************

TAO_GIOP_ReplyStatusType
TAO_GIOP_Twoway_Invocation::invoke (CORBA::ExceptionList &exceptions,
                                    CORBA::Environment &env)
{
  TAO_GIOP_ReplyStatusType retval =
    TAO_GIOP_Invocation::invoke (1, env);
  TAO_CHECK_ENV_RETURN (env, retval);
  ACE_UNUSED_ARG (retval);

  // This blocks until the response is read.  In the current version,
  // there is only one client thread that ever uses this connection,
  // so most response messages are illegal.
  //
  // THREADING NOTE: to make more efficient use of connection
  // resources, we'd multiplex I/O on connections.  For example, one
  // thread would write its GIOP::Request (or GIOP::LocateRequest etc)
  // message and block for the response, then another would do the
  // same thing.  When a response came back, it would be handed to the
  // thread which requested it.
  //
  // Currently the connection manager doesn't support such fine
  // grained connection locking, and also this server implementation
  // wouldn't take advantage of that potential concurrency in requests
  // either.  There are often performance losses coming from
  // fine-grained locks being used inappropriately; there's some
  // evidence that locking at the level of requests loses on at least
  // some platforms.
  //
  // @@ In all MT environments, there's a cancellation point lurking
  // here; need to investigate.  Client threads would frequently be
  // canceled sometime during recv_request ... the correct action to
  // take on being canceled is to issue a CancelRequest message to the
  // server and then imediately let other client-side cancellation
  // handlers do their jobs.
  //
  // In C++, that basically means to unwind the stack using almost
  // normal procedures: all destructors should fire, and some "catch"
  // blocks should probably be able to handle things like releasing
  // pointers. (Without unwinding the C++ stack, resources that must
  // be freed by thread cancellation won't be freed, and the process
  // won't continue to function correctly.)  The tricky part is that
  // according to POSIX, all C stack frames must also have their
  // (explicitly coded) handlers called.  We assume a POSIX.1c/C/C++
  // environment.

  TAO_SVC_HANDLER *handler = this->data_->handler ();
  TAO_GIOP::Message_Type m = TAO_GIOP::recv_request (handler,
                                                     this->inp_stream_,
                                                     this->orb_core_);

  this->orb_core_->reactor ()->resume_handler (this->data_->handler ());
  // suspend was called in TAO_Client_Connection_Handler::handle_input

  switch (m)
    {
    case TAO_GIOP::Reply:
      // handle reply ... must be right one etc
      break;

    case TAO_GIOP::CloseConnection:
      return (TAO_GIOP_Invocation::close_connection ());

    case TAO_GIOP::Request:
    case TAO_GIOP::CancelRequest:
    case TAO_GIOP::LocateRequest:
    case TAO_GIOP::LocateReply:
    default:
      // These are all illegal messages to find.  If found, they could
      // be indicative of client bugs (lost track of input stream) or
      // server bugs; maybe the request was acted on, maybe not, we
      // can't tell.
      ACE_DEBUG ((LM_DEBUG,
                  "(%P|%t) illegal GIOP message (%s) in response to my Request!\n",
                  TAO_GIOP::message_name (m)));
      // FALLTHROUGH ...

    case TAO_GIOP::CommunicationError:
    case TAO_GIOP::MessageError:
      // Couldn't read it for some reason ... exception's set already,
      // so just tell the other end about the trouble (closing the
      // connection) and return.

      // FALLTHROUGH

    case TAO_GIOP::EndOfFile:
      // @@ This should only refer to "getting GIOP MessageError" message only.
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  // Process reply message.  Again, due to the single threading in
  // this code, only the reply to this request is allowed to be coming
  // back.
  //
  // NOTE: if the response really _isn't_ for this thread, it's now
  // treated as an error in which synchronization can't be recovered.
  // There might be cases where it _could_ be recovered ... e.g. maybe
  // for some reason the previous call couldn't pick up its response.
  // It'd be worth investigating (and handling) any such cases.
  //
  // NOTE: since this implementation supports no ORB services
  // (notably, the transaction service, which is the only one that's
  // currently defined), the reply context is discarded.  Normally
  // it'd be fed, component at a time, to the relevant services.
  //
  // NOTE: As security support kicks in, this is the right place to
  // verify a digital signature, if that is required in this
  // particular runtime security environment.  How to know if that's
  // the case?  It's likely that standard Internet IPSEC
  // infrastructure (RFC 1825 through 1827, and successors) will be
  // used to enforce many security policies; integrity and privacy
  // guarantees may be provided by the network, and need no support
  // here.

  TAO_GIOP_ServiceContextList reply_ctx;
  CORBA::ULong request_id;
  CORBA::ULong reply_status;            // TAO_GIOP_ReplyStatusType

  this->inp_stream_ >> reply_ctx;
  if (!this->inp_stream_.good_bit ())
    {
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::MARSHAL (CORBA::COMPLETED_NO), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  if (!this->inp_stream_.read_ulong (request_id)
      || request_id != this->my_request_id_
      || !this->inp_stream_.read_ulong (reply_status)
      || reply_status > TAO_GIOP_LOCATION_FORWARD)
    {
      this->data_->handler ()->handle_close ();
      ACE_DEBUG ((LM_DEBUG, "(%P|%t) bad Response header\n"));
      TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  // If there was no exception, let the caller parse the normal
  // response.  Otherwise parse and handle the response; we always
  // know how to deal with the standard exceptions, and the caller
  // provides a list of allowed user-defined exceptions so that we
  // know how to unmarshal those too (without IFR consultation).
  //
  // When requests are forwarded, we just store the revised profile
  // data in this objref structure.  The expectation is that the call
  // will be reissued until someone gives up on a forwarding chain,
  // and that other calls will reap the benefit of the forwarding work
  // by this thread.
  //
  // NOTE: should ensure that from here on, all system exceptions
  // return COMPLETED_YES status ... even ones reported by code which
  // we call.

  switch (reply_status)
    {
    case TAO_GIOP_NO_EXCEPTION:
      break;

    case TAO_GIOP_USER_EXCEPTION:
    case TAO_GIOP_SYSTEM_EXCEPTION:
      {
        // @@ TODO This code is not exception safe. Notice how on
        // every exit path we have to invoke TAO_GIOP::send_error,
        // this should be handled by the destructor of some class;
        // which is disabled on the normal exit paths.
        // Plus <buf> should be stored in a CORBA::String_var

        char* buf;
        CORBA::String_var buf_holder; // Used to clean up dynamic allocated string
                                      // in <buf>.

        // Pull the exception ID out of the marshaling buffer.
        {
          if (this->inp_stream_.read_string (buf) == 0)
            {
              this->data_->handler ()->handle_close ();
              TAO_THROW_ENV_RETURN (CORBA::MARSHAL (CORBA::COMPLETED_YES), env, TAO_GIOP_SYSTEM_EXCEPTION);
            }
        }

        buf_holder = buf;       // Assume ownership of <buf>

        if (reply_status == TAO_GIOP_SYSTEM_EXCEPTION)
          {
            CORBA_Exception *exception =
              TAO_Exceptions::create_system_exception (buf_holder.in (), env);
            TAO_CHECK_ENV_RETURN (env, TAO_GIOP_SYSTEM_EXCEPTION);

            if (exception != 0)
              {
                this->inp_stream_.decode (exception->_type (),
                                          &exception, 0,
                                          env);
                TAO_CHECK_ENV_RETURN (env, TAO_GIOP_SYSTEM_EXCEPTION)
;
                env.exception (exception);
                return TAO_GIOP_SYSTEM_EXCEPTION;
              }
            else
              {
                // @@ TODO We should have policies to handle this
                // error, for instance:
                // + the spec requires us to silently raise a
                // CORBA::UNKNOWN exception
                // + Don't print a message and try
                // + Print the message and try
                // + Print the message and rasize CORBA::UNKNOWN
                ACE_ERROR ((LM_ERROR,
                            "Received Reply with SYSTEM_EXCEPTION "
                            "status, but unknown or invalid "
                            "exception.\n"
                            "Trying to interpret as a user exception"));
              }
          }
          {
            // Find it in the operation description and then use that
            // to get the typecode.
            // This is important to decode the exception.

            for (CORBA::ULong i = 0;
                 i < exceptions.count ();
                 i++)
              {
                CORBA::TypeCode_ptr tcp = 0;
                int loop_continue = 0;
                TAO_TRY
                  {
                    tcp = exceptions.item (i, env);
                    TAO_CHECK_ENV;

                    const char *xid = tcp->id (env);
                    TAO_CHECK_ENV;

                    if (ACE_OS::strcmp (buf_holder.in (), xid) != 0)
                      loop_continue = 1;
                  }
                TAO_CATCH (CORBA_SystemException, ex)
                  {
                    this->data_->handler ()->handle_close ();
                    TAO_RETHROW_SAME_ENV_RETURN (TAO_GIOP_SYSTEM_EXCEPTION);
                  }
                TAO_ENDTRY;
                if (loop_continue)
                  continue;

                const ACE_Message_Block* cdr =
                  this->inp_stream_.start ();
                CORBA_Any any (tcp, cdr);
                CORBA_Exception *exception =
                  new CORBA_UnknownUserException (any);
                env.exception (exception);
                return TAO_GIOP_USER_EXCEPTION;
              }
          }

        // If we couldn't find the right exception, report it as
        // CORBA::UNKNOWN.

        TAO_THROW_ENV_RETURN (CORBA::UNKNOWN (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
      }
    // NOTREACHED

    case TAO_GIOP_LOCATION_FORWARD:
      return (this->location_forward (this->inp_stream_, env));
    }

  // All standard exceptions from here on in the call path know for
  // certain that the call "completed" ... except in the case of
  // system exceptions which say otherwise, and for
  // TAO_GIOP_LOCATION_FORWARD responses.

  return (TAO_GIOP_ReplyStatusType) reply_status;
}

// Send request, block until any reply comes back, and unmarshal reply
// parameters as appropriate.
//
// This invoke method is for the stubs to use

TAO_GIOP_ReplyStatusType
TAO_GIOP_Twoway_Invocation::invoke (TAO_Exception_Data *excepts,
                                    CORBA::ULong except_count,
                                    CORBA::Environment &env)
{
  TAO_GIOP_ReplyStatusType retval =
    TAO_GIOP_Invocation::invoke (1, env);
  TAO_CHECK_ENV_RETURN (env, retval);
  ACE_UNUSED_ARG (retval);

  // This blocks until the response is read.  In the current version,
  // there is only one client thread that ever uses this connection,
  // so most response messages are illegal.
  //
  // THREADING NOTE: to make more efficient use of connection
  // resources, we'd multiplex I/O on connections.  For example, one
  // thread would write its GIOP::Request (or GIOP::LocateRequest etc)
  // message and block for the response, then another would do the
  // same thing.  When a response came back, it would be handed to the
  // thread which requested it.
  //
  // Currently the connection manager doesn't support such fine
  // grained connection locking, and also this server implementation
  // wouldn't take advantage of that potential concurrency in requests
  // either.  There are often performance losses coming from
  // fine-grained locks being used inappropriately; there's some
  // evidence that locking at the level of requests loses on at least
  // some platforms.
  //
  // @@ In all MT environments, there's a cancellation point lurking
  // here; need to investigate.  Client threads would frequently be
  // canceled sometime during recv_request ... the correct action to
  // take on being canceled is to issue a CancelRequest message to the
  // server and then imediately let other client-side cancellation
  // handlers do their jobs.
  //
  // In C++, that basically means to unwind the stack using almost
  // normal procedures: all destructors should fire, and some "catch"
  // blocks should probably be able to handle things like releasing
  // pointers. (Without unwinding the C++ stack, resources that must
  // be freed by thread cancellation won't be freed, and the process
  // won't continue to function correctly.)  The tricky part is that
  // according to POSIX, all C stack frames must also have their
  // (explicitly coded) handlers called.  We assume a POSIX.1c/C/C++
  // environment.

  TAO_SVC_HANDLER *handler = this->data_->handler ();
  TAO_GIOP::Message_Type m = TAO_GIOP::recv_request (handler,
                                                     this->inp_stream_,
                                                     this->orb_core_);

  this->orb_core_->reactor ()->resume_handler (this->data_->handler ());
  // suspend was called in TAO_Client_Connection_Handler::handle_input

  switch (m)
    {
    case TAO_GIOP::Reply:
      // handle reply ... must be right one etc
      break;

    case TAO_GIOP::CloseConnection:
      return (TAO_GIOP_Invocation::close_connection ());

    case TAO_GIOP::Request:
    case TAO_GIOP::CancelRequest:
    case TAO_GIOP::LocateRequest:
    case TAO_GIOP::LocateReply:
    default:
      // These are all illegal messages to find.  If found, they could
      // be indicative of client bugs (lost track of input stream) or
      // server bugs; maybe the request was acted on, maybe not, we
      // can't tell.
      ACE_DEBUG ((LM_DEBUG,
                  "(%P|%t) illegal GIOP message (%s) in response to my Request!\n",
                  TAO_GIOP::message_name (m)));
      // FALLTHROUGH ...

    case TAO_GIOP::CommunicationError:
    case TAO_GIOP::MessageError:
      // Couldn't read it for some reason ... exception's set already,
      // so just tell the other end about the trouble (closing the
      // connection) and return.

      // FALLTHROUGH

    case TAO_GIOP::EndOfFile:
      // @@ This should only refer to "getting GIOP MessageError" message only.
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  // Process reply message.  Again, due to the single threading in
  // this code, only the reply to this request is allowed to be coming
  // back.
  //
  // NOTE: if the response really _isn't_ for this thread, it's now
  // treated as an error in which synchronization can't be recovered.
  // There might be cases where it _could_ be recovered ... e.g. maybe
  // for some reason the previous call couldn't pick up its response.
  // It'd be worth investigating (and handling) any such cases.
  //
  // NOTE: since this implementation supports no ORB services
  // (notably, the transaction service, which is the only one that's
  // currently defined), the reply context is discarded.  Normally
  // it'd be fed, component at a time, to the relevant services.
  //
  // NOTE: As security support kicks in, this is the right place to
  // verify a digital signature, if that is required in this
  // particular runtime security environment.  How to know if that's
  // the case?  It's likely that standard Internet IPSEC
  // infrastructure (RFC 1825 through 1827, and successors) will be
  // used to enforce many security policies; integrity and privacy
  // guarantees may be provided by the network, and need no support
  // here.

  TAO_GIOP_ServiceContextList reply_ctx;
  CORBA::ULong request_id;
  CORBA::ULong reply_status;            // TAO_GIOP_ReplyStatusType

  this->inp_stream_ >> reply_ctx;
  if (!this->inp_stream_.good_bit ())
    {
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::MARSHAL (CORBA::COMPLETED_NO), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  if (!this->inp_stream_.read_ulong (request_id)
      || request_id != this->my_request_id_
      || !this->inp_stream_.read_ulong (reply_status)
      || reply_status > TAO_GIOP_LOCATION_FORWARD)
    {
      this->data_->handler ()->handle_close ();
      ACE_DEBUG ((LM_DEBUG, "(%P|%t) bad Response header\n"));
      TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  // If there was no exception, let the caller parse the normal
  // response.  Otherwise parse and handle the response; we always
  // know how to deal with the standard exceptions, and the caller
  // provides a list of allowed user-defined exceptions so that we
  // know how to unmarshal those too (without IFR consultation).
  //
  // When requests are forwarded, we just store the revised profile
  // data in this objref structure.  The expectation is that the call
  // will be reissued until someone gives up on a forwarding chain,
  // and that other calls will reap the benefit of the forwarding work
  // by this thread.
  //
  // NOTE: should ensure that from here on, all system exceptions
  // return COMPLETED_YES status ... even ones reported by code which
  // we call.

  switch (reply_status)
    {
    case TAO_GIOP_NO_EXCEPTION:
      break;

    case TAO_GIOP_USER_EXCEPTION:
    case TAO_GIOP_SYSTEM_EXCEPTION:
      {
        // @@ TODO This code is not exception safe. Notice how on
        // every exit path we have to invoke TAO_GIOP::send_error,
        // this should be handled by the destructor of some class;
        // which is disabled on the normal exit paths.
        // Plus <buf> should be stored in a CORBA::String_var

        char* buf;
        CORBA::String_var buf_holder; // Used to clean up dynamically allocated
                                      // <buf> upon exceptions or return

        // Pull the exception ID out of the marshaling buffer.
        {
          if (this->inp_stream_.read_string (buf) == 0)
            {
              this->data_->handler ()->handle_close ();
              TAO_THROW_ENV_RETURN (CORBA::MARSHAL (CORBA::COMPLETED_YES), env, TAO_GIOP_SYSTEM_EXCEPTION);
            }
        }

        buf_holder = buf;       // Assume ownership of <buf>

        // Find it in the operation description and then use that to
        // get the typecode.  Use it to unmarshal the exception's
        // value; if that exception is not allowed by this operation,
        // fail (next).

        // placeholder to decode the exception
        CORBA::Exception *exception = 0;
        CORBA::TypeCode_ptr tcp = 0;

        if (reply_status == TAO_GIOP_SYSTEM_EXCEPTION)
          {
            exception =
              TAO_Exceptions::create_system_exception (buf_holder.in (), env);
            TAO_CHECK_ENV_RETURN (env, TAO_GIOP_SYSTEM_EXCEPTION);

            if (exception != 0)
              {
                this->inp_stream_.decode (exception->_type (),
                                          exception, 0,
                                          env);
                TAO_CHECK_ENV_RETURN (env, TAO_GIOP_SYSTEM_EXCEPTION);

                // @@ What do we do if an exception is raised while
                // demarshaling an exception????
                env.exception (exception);
                return TAO_GIOP_SYSTEM_EXCEPTION;
              }
          }
        // else // this else is commented out, see the coment above
          {
            // search the table of exceptions and see if there is a match
            for (CORBA::ULong i = 0;
                 i < except_count;
                 i++)
              {
                int loop_continue = 0;
                TAO_TRY
                  {
                    tcp = excepts[i].tc;
                    const char *xid = tcp->id (env);
                    TAO_CHECK_ENV;

                    if (ACE_OS::strcmp (buf_holder.in (), (char *)xid) != 0)
                      loop_continue = 1;
                    else
                      {
                        // match
                        exception = excepts[i].alloc ();

                        this->inp_stream_.decode (exception->_type (),
                                                  exception, 0,
                                                  env);
                        TAO_CHECK_ENV;
                      }
                  }
                TAO_CATCH (CORBA_SystemException, ex)
                  {
                    this->data_->handler ()->handle_close ();
                    TAO_RETHROW_SAME_ENV_RETURN (TAO_GIOP_SYSTEM_EXCEPTION);
                  }
                TAO_ENDTRY;
                if (loop_continue)
                  continue;

                env.exception (exception);
                return TAO_GIOP_USER_EXCEPTION;
              } // end of loop
          }

        // If we couldn't find the right exception, report it as
        // CORBA::UNKNOWN.

        env.exception (new CORBA::UNKNOWN (CORBA::COMPLETED_YES));
        return TAO_GIOP_SYSTEM_EXCEPTION;
      }
    // NOTREACHED

    case TAO_GIOP_LOCATION_FORWARD:
      return (this->location_forward (this->inp_stream_, env));
    }

  // All standard exceptions from here on in the call path know for
  // certain that the call "completed" ... except in the case of
  // system exceptions which say otherwise, and for
  // TAO_GIOP_LOCATION_FORWARD responses.

  return (TAO_GIOP_ReplyStatusType) reply_status;
}


// ****************************************************************


// Send request, block until any reply comes back

TAO_GIOP_ReplyStatusType
TAO_GIOP_Locate_Request_Invocation::invoke (CORBA::Environment &env)
{
  // Send Request, return on error or if we're done

  if (this->data_->handler ()->send_request (this->orb_core_,
                                             this->out_stream_,
                                             1) == -1)
    {
      // send_request () closed the connection; we just set the
      // handler to 0 here.
      this->data_->reset_handler ();
      env.exception (new CORBA::TRANSIENT (CORBA::COMPLETED_MAYBE));
      return TAO_GIOP_SYSTEM_EXCEPTION;
    }

  TAO_SVC_HANDLER *handler = this->data_->handler ();
  TAO_GIOP::Message_Type m = TAO_GIOP::recv_request (handler,
                                                     this->inp_stream_,
                                                     this->orb_core_);

  this->orb_core_->reactor ()->resume_handler (this->data_->handler ());
  // suspend was called in TAO_Client_Connection_Handler::handle_input

  switch (m)
    {
    case TAO_GIOP::CloseConnection:
      return (this->close_connection ());

    case TAO_GIOP::LocateReply:
      // Handle the reply
      // Especially set the new location

      CORBA::ULong request_id;
      CORBA::ULong locate_status;      // TAO_GIOP_LocateStatusType

      if (!this->inp_stream_.read_ulong (request_id)
          || request_id != this->my_request_id_
          || !this->inp_stream_.read_ulong (locate_status))
        {
          this->data_->handler ()->handle_close ();
          ACE_DEBUG ((LM_DEBUG,
                      "(%P|%t) bad Response header\n"));
          TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
        }
      switch (locate_status)
        {
        case TAO_GIOP_UNKNOWN_OBJECT:
          TAO_THROW_ENV_RETURN (CORBA::OBJECT_NOT_EXIST (CORBA::COMPLETED_YES), env, TAO_GIOP_SYSTEM_EXCEPTION);
          /* not reached */
        case TAO_GIOP_OBJECT_HERE:
          return TAO_GIOP_NO_EXCEPTION;
          /* not reached */
        case TAO_GIOP_OBJECT_FORWARD:
          return (this->location_forward (this->inp_stream_, env));
          /* not reached */
        }
      /* not reached */
    case TAO_GIOP::Reply:
    case TAO_GIOP::Request:
    case TAO_GIOP::CancelRequest:
    case TAO_GIOP::LocateRequest:
    default:
      // These are all illegal messages to find.  If found, they could
      // be indicative of client bugs (lost track of input stream) or
      // server bugs; maybe the request was acted on, maybe not, we
      // can't tell.
      ACE_DEBUG ((LM_DEBUG,
                  "(%P|%t) illegal GIOP message (%s) in response to my Request!\n",
                  TAO_GIOP::message_name (m)));
      // FALLTHROUGH ...

    case TAO_GIOP::CommunicationError:
    case TAO_GIOP::MessageError:
      // Couldn't read it for some reason ... exception's set already,
      // so just tell the other end about the trouble (closing the
      // connection) and return.
      // FALLTHROUGH

    case TAO_GIOP::EndOfFile:
      // @@ This should only refer to "getting GIOP MessageError" message only.
      this->data_->handler ()->handle_close ();
      TAO_THROW_ENV_RETURN (CORBA::COMM_FAILURE (CORBA::COMPLETED_MAYBE), env, TAO_GIOP_SYSTEM_EXCEPTION);
    }

  ACE_NOTREACHED (return TAO_GIOP_NO_EXCEPTION);
}

// ****************************************************************

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
