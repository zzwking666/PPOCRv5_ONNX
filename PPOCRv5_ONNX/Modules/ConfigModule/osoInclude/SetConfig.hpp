#pragma once

#include"oso/oso_core.h"
#include <string>

namespace cdm {
    class SetConfig
    {
    public:
        SetConfig() = default;
        ~SetConfig() = default;

        SetConfig(const rw::oso::ObjectStoreAssembly& assembly);
        SetConfig(const SetConfig& obj);

        SetConfig& operator=(const SetConfig& obj);
        operator rw::oso::ObjectStoreAssembly() const;
        bool operator==(const SetConfig& obj) const;
        bool operator!=(const SetConfig& obj) const;

    public:
        int chengfaqi1{ 0 };
        int houfenpin1{ 0 };
        int zengyi1{ 0 };
        int baoguang1{ 0 };
        int chengfaqi2{ 0 };
        int houfenpin2{ 0 };
        int zengyi2{ 0 };
        int baoguang2{ 0 };
        double xuantingshijian{ 0 };
        int qiehuanzhangshu{ 0 };
        bool isjingxiang{ false };
        double youyijuli{ 0 };
        double suofang{ 0 };
    };

    inline SetConfig::SetConfig(const rw::oso::ObjectStoreAssembly& assembly)
    {
        auto isAccountAssembly = assembly.getName();
        if (isAccountAssembly != "$class$SetConfig$")
        {
            throw std::runtime_error("Assembly is not $class$SetConfig$");
        }
        auto chengfaqi1Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$chengfaqi1$"));
        if (!chengfaqi1Item) {
            throw std::runtime_error("$variable$chengfaqi1 is not found");
        }
        chengfaqi1 = chengfaqi1Item->getValueAsInt();
        auto houfenpin1Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$houfenpin1$"));
        if (!houfenpin1Item) {
            throw std::runtime_error("$variable$houfenpin1 is not found");
        }
        houfenpin1 = houfenpin1Item->getValueAsInt();
        auto zengyi1Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$zengyi1$"));
        if (!zengyi1Item) {
            throw std::runtime_error("$variable$zengyi1 is not found");
        }
        zengyi1 = zengyi1Item->getValueAsInt();
        auto baoguang1Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$baoguang1$"));
        if (!baoguang1Item) {
            throw std::runtime_error("$variable$baoguang1 is not found");
        }
        baoguang1 = baoguang1Item->getValueAsInt();
        auto chengfaqi2Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$chengfaqi2$"));
        if (!chengfaqi2Item) {
            throw std::runtime_error("$variable$chengfaqi2 is not found");
        }
        chengfaqi2 = chengfaqi2Item->getValueAsInt();
        auto houfenpin2Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$houfenpin2$"));
        if (!houfenpin2Item) {
            throw std::runtime_error("$variable$houfenpin2 is not found");
        }
        houfenpin2 = houfenpin2Item->getValueAsInt();
        auto zengyi2Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$zengyi2$"));
        if (!zengyi2Item) {
            throw std::runtime_error("$variable$zengyi2 is not found");
        }
        zengyi2 = zengyi2Item->getValueAsInt();
        auto baoguang2Item = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$baoguang2$"));
        if (!baoguang2Item) {
            throw std::runtime_error("$variable$baoguang2 is not found");
        }
        baoguang2 = baoguang2Item->getValueAsInt();
        auto xuantingshijianItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$xuantingshijian$"));
        if (!xuantingshijianItem) {
            throw std::runtime_error("$variable$xuantingshijian is not found");
        }
        xuantingshijian = xuantingshijianItem->getValueAsDouble();
        auto qiehuanzhangshuItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$qiehuanzhangshu$"));
        if (!qiehuanzhangshuItem) {
            throw std::runtime_error("$variable$qiehuanzhangshu is not found");
        }
        qiehuanzhangshu = qiehuanzhangshuItem->getValueAsInt();
        auto isjingxiangItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$isjingxiang$"));
        if (!isjingxiangItem) {
            throw std::runtime_error("$variable$isjingxiang is not found");
        }
        isjingxiang = isjingxiangItem->getValueAsBool();
        auto youyijuliItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$youyijuli$"));
        if (!youyijuliItem) {
            throw std::runtime_error("$variable$youyijuli is not found");
        }
        youyijuli = youyijuliItem->getValueAsDouble();
        auto suofangItem = rw::oso::ObjectStoreCoreToItem(assembly.getItem("$variable$suofang$"));
        if (!suofangItem) {
            throw std::runtime_error("$variable$suofang is not found");
        }
        suofang = suofangItem->getValueAsDouble();
    }

    inline SetConfig::SetConfig(const SetConfig& obj)
    {
        chengfaqi1 = obj.chengfaqi1;
        houfenpin1 = obj.houfenpin1;
        zengyi1 = obj.zengyi1;
        baoguang1 = obj.baoguang1;
        chengfaqi2 = obj.chengfaqi2;
        houfenpin2 = obj.houfenpin2;
        zengyi2 = obj.zengyi2;
        baoguang2 = obj.baoguang2;
        xuantingshijian = obj.xuantingshijian;
        qiehuanzhangshu = obj.qiehuanzhangshu;
        isjingxiang = obj.isjingxiang;
        youyijuli = obj.youyijuli;
        suofang = obj.suofang;
    }

    inline SetConfig& SetConfig::operator=(const SetConfig& obj)
    {
        if (this != &obj) {
            chengfaqi1 = obj.chengfaqi1;
            houfenpin1 = obj.houfenpin1;
            zengyi1 = obj.zengyi1;
            baoguang1 = obj.baoguang1;
            chengfaqi2 = obj.chengfaqi2;
            houfenpin2 = obj.houfenpin2;
            zengyi2 = obj.zengyi2;
            baoguang2 = obj.baoguang2;
            xuantingshijian = obj.xuantingshijian;
            qiehuanzhangshu = obj.qiehuanzhangshu;
            isjingxiang = obj.isjingxiang;
            youyijuli = obj.youyijuli;
            suofang = obj.suofang;
        }
        return *this;
    }

    inline SetConfig::operator rw::oso::ObjectStoreAssembly() const
    {
        rw::oso::ObjectStoreAssembly assembly;
        assembly.setName("$class$SetConfig$");
        auto chengfaqi1Item = std::make_shared<rw::oso::ObjectStoreItem>();
        chengfaqi1Item->setName("$variable$chengfaqi1$");
        chengfaqi1Item->setValueFromInt(chengfaqi1);
        assembly.addItem(chengfaqi1Item);
        auto houfenpin1Item = std::make_shared<rw::oso::ObjectStoreItem>();
        houfenpin1Item->setName("$variable$houfenpin1$");
        houfenpin1Item->setValueFromInt(houfenpin1);
        assembly.addItem(houfenpin1Item);
        auto zengyi1Item = std::make_shared<rw::oso::ObjectStoreItem>();
        zengyi1Item->setName("$variable$zengyi1$");
        zengyi1Item->setValueFromInt(zengyi1);
        assembly.addItem(zengyi1Item);
        auto baoguang1Item = std::make_shared<rw::oso::ObjectStoreItem>();
        baoguang1Item->setName("$variable$baoguang1$");
        baoguang1Item->setValueFromInt(baoguang1);
        assembly.addItem(baoguang1Item);
        auto chengfaqi2Item = std::make_shared<rw::oso::ObjectStoreItem>();
        chengfaqi2Item->setName("$variable$chengfaqi2$");
        chengfaqi2Item->setValueFromInt(chengfaqi2);
        assembly.addItem(chengfaqi2Item);
        auto houfenpin2Item = std::make_shared<rw::oso::ObjectStoreItem>();
        houfenpin2Item->setName("$variable$houfenpin2$");
        houfenpin2Item->setValueFromInt(houfenpin2);
        assembly.addItem(houfenpin2Item);
        auto zengyi2Item = std::make_shared<rw::oso::ObjectStoreItem>();
        zengyi2Item->setName("$variable$zengyi2$");
        zengyi2Item->setValueFromInt(zengyi2);
        assembly.addItem(zengyi2Item);
        auto baoguang2Item = std::make_shared<rw::oso::ObjectStoreItem>();
        baoguang2Item->setName("$variable$baoguang2$");
        baoguang2Item->setValueFromInt(baoguang2);
        assembly.addItem(baoguang2Item);
        auto xuantingshijianItem = std::make_shared<rw::oso::ObjectStoreItem>();
        xuantingshijianItem->setName("$variable$xuantingshijian$");
        xuantingshijianItem->setValueFromDouble(xuantingshijian);
        assembly.addItem(xuantingshijianItem);
        auto qiehuanzhangshuItem = std::make_shared<rw::oso::ObjectStoreItem>();
        qiehuanzhangshuItem->setName("$variable$qiehuanzhangshu$");
        qiehuanzhangshuItem->setValueFromInt(qiehuanzhangshu);
        assembly.addItem(qiehuanzhangshuItem);
        auto isjingxiangItem = std::make_shared<rw::oso::ObjectStoreItem>();
        isjingxiangItem->setName("$variable$isjingxiang$");
        isjingxiangItem->setValueFromBool(isjingxiang);
        assembly.addItem(isjingxiangItem);
        auto youyijuliItem = std::make_shared<rw::oso::ObjectStoreItem>();
        youyijuliItem->setName("$variable$youyijuli$");
        youyijuliItem->setValueFromDouble(youyijuli);
        assembly.addItem(youyijuliItem);
        auto suofangItem = std::make_shared<rw::oso::ObjectStoreItem>();
        suofangItem->setName("$variable$suofang$");
        suofangItem->setValueFromDouble(suofang);
        assembly.addItem(suofangItem);
        return assembly;
    }

    inline bool SetConfig::operator==(const SetConfig& obj) const
    {
        return chengfaqi1 == obj.chengfaqi1 && houfenpin1 == obj.houfenpin1 && zengyi1 == obj.zengyi1 && baoguang1 == obj.baoguang1 && chengfaqi2 == obj.chengfaqi2 && houfenpin2 == obj.houfenpin2 && zengyi2 == obj.zengyi2 && baoguang2 == obj.baoguang2 && xuantingshijian == obj.xuantingshijian && qiehuanzhangshu == obj.qiehuanzhangshu && isjingxiang == obj.isjingxiang && youyijuli == obj.youyijuli && suofang == obj.suofang;
    }

    inline bool SetConfig::operator!=(const SetConfig& obj) const
    {
        return !(*this == obj);
    }

}

