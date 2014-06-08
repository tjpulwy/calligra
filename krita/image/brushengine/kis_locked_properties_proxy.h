#ifndef KIS_LOCKED_PROPERTIES_PROXY_H
#define KIS_LOCKED_PROPERTIES_PROXY_H

#include "kis_locked_properties.h"
#include "kis_properties_configuration.h"

class KisLockedPropertiesProxy: public KisPropertiesConfiguration
{
public:
    KisLockedPropertiesProxy() ;
    KisLockedPropertiesProxy(KisLockedProperties* p);
    KisLockedPropertiesProxy(KisPropertiesConfiguration *, KisLockedProperties *);
    bool getBool(const QString &name, bool def) const;



private:
    KisLockedProperties* m_lockedProperties;
    KisPropertiesConfiguration* m_parent;

};

#endif // KIS_LOCKED_PROPERTIES_PROXY_H
