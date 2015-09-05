#include "component_helper.h"
#include <stdio.h>
#include "task_engine.h"
#include "common_macros.h"

// for test
#include "test_component_1.h"
#include "test_component_2.h"

namespace platform
{
ComponentHelper* ComponentHelper::s_helper_ = NULL;
pthread_once_t ComponentHelper::s_once_ = PTHREAD_ONCE_INIT;
ComponentHelper::ComponentHelper(){}
ComponentHelper::~ComponentHelper(){}

void ComponentHelper::Init()
{
    s_helper_ = new ComponentHelper;
}

ComponentHelper* ComponentHelper::GetInstance()
{
    pthread_once(&s_once_, Init);
    return s_helper_;
}

void ComponentHelper::Release()
{
    delete s_helper_;
    s_helper_ = NULL;
}

int ComponentHelper::LoadAllComponents(const std::string& conf_file)
{
    // fake components
    Component* component = new Component1;
    component->set_component_id(1);
    components_.insert(std::make_pair(component->get_component_id(), component));
    component = new Component2;
    component->set_component_id(2);
    components_.insert(std::make_pair(component->get_component_id(), component));
    return 0;
}

int ComponentHelper::InstallAllComponents()
{
    component_itor itor = components_.begin();
    for (; itor != components_.end(); ++itor)
    {
        TaskEngine* engine = new TaskEngine;
        if (!engine) continue;
        engine->BindComponent(itor->second);
        MsgSchedulerInstance()->Regist(engine);
    }

    if (InitComponents() != 0)
    {
        UninstallAllComponents();
        return -1;
    }

    if (ActivateComponents() != 0)
    {
        UninstallAllComponents();
        return -1;
    }

    MsgSchedulerInstance()->Schedule();
    return 0;
}

int ComponentHelper::InitComponents()
{
    component_itor itor = components_.begin();
    for (; itor != components_.end(); ++itor)
    {
        if(itor->second && itor->second->Init() != 0)
        {
            printf("component %d init failed\n", itor->first);
            return -1;
        }
    }
    return 0;
}

int ComponentHelper::ActivateComponents()
{
    component_itor itor = components_.begin();
    for (; itor != components_.end(); ++itor)
    {
        if(itor->second)
        {
            if (itor->second->Active() != 0)
            {
                printf("component %d active failed\n", itor->first);
                return -1;
            }            
        }
        else
        {
            printf("component is null, active failed\n");
            return -1;
        }
    }
    return 0;
}

int ComponentHelper::UninstallAllComponents()
{
    MsgScheduler* scheduler = MsgScheduler::GetInstance();
    if (!scheduler) return -1;
    component_itor itor = components_.begin();
    for (; itor != components_.end(); ++itor)
    {
        scheduler->UnRegist(itor->first);
        Component* component = itor->second;
        if (component)
        {
            component->Fini();
            delete component;
        }
    }
    components_.clear();
    return 0;
}

Component* ComponentHelper::GetComponent(uint8_t component_id)
{
    component_itor itor = components_.find(component_id);
    if(itor != components_.end())
    {
        return itor->second;
    }
    return NULL;
}
}
