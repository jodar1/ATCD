// $Id$
#include "Plan_Launcher_Impl.h"
#include "orbsvcs/CosNamingC.h"
#include "tools/Config_Handlers/XML_File_Intf.h"
#include "tools/Config_Handlers/DnC_Dump.h"
//#include "tao/RTCORBA/RTCORBA.h"
//#include "tao/RTCORBA/Priority_Mapping_Manager.h"
#include "tao/ORB_Core.h"
#include "ace/Sched_Params.h"
#include "ccm/CCM_ObjectC.h"

#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "DAnCE/Logger/Log_Macros.h"

namespace DAnCE
{
namespace Plan_Launcher
{

::Deployment::DeploymentPlan *
Plan_Launcher_Impl::load_xml_plan(const char *deployment_plan_uri)
{
  DANCE_DEBUG ((LM_TRACE, DLINFO "Plan_Launcher_Impl::load_xml_plan - "
                "Parsing plan \"%s\"...\n", deployment_plan_uri));

  ::Deployment::DeploymentPlan * plan = 0;
  try
    {
      CIAO::Config_Handlers::XML_File_Intf intf (deployment_plan_uri);
      intf.add_search_path ("CIAO_ROOT", "/docs/schema/");
      intf.add_search_path ("TAO_ROOT", "/docs/schema/");
      plan = intf.release_plan ();
      if (0 == plan)
        {
          DANCE_ERROR ((LM_ERROR, DLINFO "Plan_Launcher_Impl::load_xml_plan - "
                        "Failed to parse plan \"%C\".\n", deployment_plan_uri));
          throw Deployment_Failure("Failed to parse plan.");
        }
    }
  catch (...)
    {
      ACE_CString s = "failed to parse deployment plan \"";
      s += deployment_plan_uri;
      s += "\"";
      DANCE_DEBUG ((LM_ERROR, DLINFO "Plan_Launcher_Impl::load_xml_plan - %C\n", s.c_str()));
      throw Deployment_Failure (s.c_str());
    }

  DANCE_DEBUG ((LM_TRACE, DLINFO "Plan_Launcher_Impl::load_xml_plan - Parsing complete....\n"));
  return plan;
}

void Plan_Launcher_Impl::execute()
{
  DANCE_TRACE ("Plan_Launcher_Impl::execute()");

  if (this->mode_ & MODE_START_PLAN)
    {
      size_t sz = this->xml_plan_urls_.size();
      for (size_t i= 0; i < sz; ++i)
        {
          DANCE_DEBUG ((LM_TRACE, DLINFO "Plan_Launcher_Impl::execute - "
                        "launching plan \"%C\"...\n", this->xml_plan_urls_[i].c_str()));
          ::Deployment::DeploymentPlan_var plan = 
              this->load_xml_plan(this->xml_plan_urls_[i].c_str());
          try
          {
            CORBA::String_var uuid = this->launch_plan (plan.in());
            if (0 == uuid.in ())
              {
                throw Deployment_Failure ("execute - Error launching plan\n");
              }
            DANCE_DEBUG ((LM_TRACE, DLINFO "Plan_Launcher_Impl::execute - "
                          "returned plan UUID is \"%s\"\n", uuid.in ()));
          }
          catch (...)
          {
            this->teardown_plan (plan->UUID.in());
            throw;
          }
        }
      this->Plan_Launcher_Base_Impl::execute();
    }

  if (this->mode_ & MODE_STOP_PLAN)
    {
      this->stop_plan();
    }

  if (this->mode_ & MODE_WRITE_CDR)
    {
      ::Deployment::DeploymentPlan_var plan = this->load_xml_plan(this->xml_plan_urls_[0].c_str());
      this->write_cdr_plan_file(this->cdr_dest_url_.c_str(), plan);
    }
}
void Plan_Launcher_Impl::stop_plan()
{
  DANCE_TRACE ("Plan_Launcher_Impl::stop_plan");
  
  if (0 < this->xml_plan_urls_.size())
    {
      size_t sz = this->xml_plan_urls_.size();
      for (size_t i= 0; i < sz; ++i)
        {
          DANCE_DEBUG((LM_TRACE, DLINFO "Plan_Launcher_Impl::stop_plan - "
                       "Stopping plan by plan file: %C\n", this->xml_plan_urls_[i].c_str()));
          ::Deployment::DeploymentPlan_var plan = this->load_xml_plan(this->xml_plan_urls_[i].c_str());
          if (!this->teardown_plan(plan->UUID.in()))
            {
              DANCE_ERROR ((LM_ERROR, DLINFO "Plan_Launcher_Impl::stop_plan - "
                            "tear down assembly failed: unknown plan uuid.\n"));
            }
        }
    }
  this->Plan_Launcher_Base_Impl::stop_plan();
}

} // Plan_Launcher
} // DAnCE