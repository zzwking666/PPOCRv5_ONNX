#pragma once

#include"oso/oso_core.h"
#include <string>

namespace cdm {
    class BagsRealTimeDisplayInfo
    {
    public:
        BagsRealTimeDisplayInfo() = default;
        ~BagsRealTimeDisplayInfo() = default;

        BagsRealTimeDisplayInfo(const rw::oso::ObjectStoreAssembly& assembly);
        BagsRealTimeDisplayInfo(const BagsRealTimeDisplayInfo& obj);

        BagsRealTimeDisplayInfo& operator=(const BagsRealTimeDisplayInfo& obj);
        operator rw::oso::ObjectStoreAssembly() const;
        bool operator==(const BagsRealTimeDisplayInfo& obj) const;
        bool operator!=(const BagsRealTimeDisplayInfo& obj) const;

    public:
        int zhengmianzongliang{ 0 };
        int beimianzongliang{ 0 };
        int qiehuanxianshi{ 0 };
    };

    inline BagsRealTimeDisplayInfo::BagsRealTimeDisplayInfo(const rw::oso::ObjectStoreAssembly& assembly)
    {
        auto isAccountAssembly = assembly.getName();
        if (isAccountAssembly != "$class$BagsRealTimeDisplayInfo$")
        {
            throw std::runtime_error("Assembly is not $class$BagsRealTimeDisplayInfo$");
        }
        auto zhengmianzongliangItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$zhengmianzongliang$"));
        if (!zhengmianzongliangItem) {
            throw std::runtime_error("$variable$zhengmianzongliang is not found");
        }
        zhengmianzongliang = zhengmianzongliangItem->getValueAsInt();
        auto beimianzongliangItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$beimianzongliang$"));
        if (!beimianzongliangItem) {
            throw std::runtime_error("$variable$beimianzongliang is not found");
        }
        beimianzongliang = beimianzongliangItem->getValueAsInt();
        auto qiehuanxianshiItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$qiehuanxianshi$"));
        if (!qiehuanxianshiItem) {
            throw std::runtime_error("$variable$qiehuanxianshi is not found");
        }
        qiehuanxianshi = qiehuanxianshiItem->getValueAsInt();
    }

    inline BagsRealTimeDisplayInfo::BagsRealTimeDisplayInfo(const BagsRealTimeDisplayInfo& obj)
    {
        zhengmianzongliang = obj.zhengmianzongliang;
        beimianzongliang = obj.beimianzongliang;
        qiehuanxianshi = obj.qiehuanxianshi;
    }

    inline BagsRealTimeDisplayInfo& BagsRealTimeDisplayInfo::operator=(const BagsRealTimeDisplayInfo& obj)
    {
        if (this != &obj) {
            zhengmianzongliang = obj.zhengmianzongliang;
            beimianzongliang = obj.beimianzongliang;
            qiehuanxianshi = obj.qiehuanxianshi;
        }
        return *this;
    }

    inline BagsRealTimeDisplayInfo::operator rw::oso::ObjectStoreAssembly() const
    {
        rw::oso::ObjectStoreAssembly assembly;
        assembly.setName("$class$BagsRealTimeDisplayInfo$");
        auto zhengmianzongliangItem = std::make_shared<rw::oso::ObjectStoreItem>();
        zhengmianzongliangItem->setName("$variable$zhengmianzongliang$");
        zhengmianzongliangItem->setValueFromInt(zhengmianzongliang);
        assembly.addItem(zhengmianzongliangItem);
        auto beimianzongliangItem = std::make_shared<rw::oso::ObjectStoreItem>();
        beimianzongliangItem->setName("$variable$beimianzongliang$");
        beimianzongliangItem->setValueFromInt(beimianzongliang);
        assembly.addItem(beimianzongliangItem);
        auto qiehuanxianshiItem = std::make_shared<rw::oso::ObjectStoreItem>();
        qiehuanxianshiItem->setName("$variable$qiehuanxianshi$");
        qiehuanxianshiItem->setValueFromInt(qiehuanxianshi);
        assembly.addItem(qiehuanxianshiItem);
        return assembly;
    }

    inline bool BagsRealTimeDisplayInfo::operator==(const BagsRealTimeDisplayInfo& obj) const
    {
        return zhengmianzongliang == obj.zhengmianzongliang && beimianzongliang == obj.beimianzongliang && qiehuanxianshi == obj.qiehuanxianshi;
    }

    inline bool BagsRealTimeDisplayInfo::operator!=(const BagsRealTimeDisplayInfo& obj) const
    {
        return !(*this == obj);
    }

}

