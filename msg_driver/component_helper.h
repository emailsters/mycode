#ifndef _COMPONENT_HELPER_H_
#define _COMPONENT_HELPER_H_
#include <iostream>
#include <map>
#include <pthread.h>
#include "class_macros.h"
#include "component.h"

namespace platform
{
class TaskEngine;

// not thread safe, just use it in main thread
class ComponentHelper
{
DISALLOW_COPY_AND_ASSIGN(ComponentHelper);
public:
    static ComponentHelper* GetInstance();
    void Release();
    int LoadAllComponents(const std::string& conf_file);
    int InstallAllComponents();
    int UninstallAllComponents();
    Component* GetComponent(uint8_t component_id);
private:
    ComponentHelper();
    ~ComponentHelper();
    static void Init();
    int InitComponents();
    int ActivateComponents();
    typedef std::map<uint8_t, Component*>::iterator component_itor;
    std::map<uint8_t, Component*> components_;
    static ComponentHelper* s_helper_;
    static pthread_once_t s_once_;
};
}

#endif
