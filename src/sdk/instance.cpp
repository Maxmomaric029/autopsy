#include "engine.h"

namespace sdk {

    static bool is_valid_instance_address(uintptr_t addr)
    {
        return addr != 0 && addr != 0xFFFFFFFFFFFFFFFF;
    }

    std::string instance::name() const {

        if (!Address) return "?";
        uintptr_t StringPointer = drive->read<uintptr_t>(Address + offset::instance::name);
        if (!StringPointer) return "?";
        return drive->readstring(StringPointer);
    }

    std::string instance::text() const {

        return drive->readstring(this->Address + offset::gui::text);
    }

    std::string instance::kind() const {

        if (!Address) return "?";
        uintptr_t Descriptor = drive->read<uintptr_t>(Address + offset::instance::ClassDescriptor);
        if (!Descriptor) return "?";
        uintptr_t StringPointer = drive->read<uintptr_t>(Descriptor + offset::instance::ClassName);
        if (!StringPointer) return "?";
        return drive->readstring(StringPointer);
    }

    instance instance::parent() const {

        if (!Address) return instance();
        return drive->read<instance>(Address + offset::instance::parent);
    }

    std::vector<instance> instance::children() const {

        std::vector<instance> Container;
        if (!Address) return Container;

        uintptr_t childStart = drive->read<uintptr_t>(Address + offset::instance::ChildrenStart);
        if (!childStart) return Container;

        uintptr_t childEnd = drive->read<uintptr_t>(childStart + offset::instance::ChildrenEnd);
        if (!childEnd || childEnd <= childStart) return Container;

        for (uintptr_t ptr = childStart; ptr < childEnd; ptr += 0x10)
        {
            uintptr_t child = drive->read<uintptr_t>(ptr);
            if (is_valid_instance_address(child))
                Container.emplace_back(drive->read<instance>(ptr));
        }

        return Container;
    }

    instance instance::child(const std::string& name) const {

        if (!Address || name.empty()) return instance();

        for (instance Child : children())
        {
            if (!Child.Address) continue;
            if (Child.name() == name) return Child;
        }

        return instance();
    }

    instance instance::childclass(const std::string& Class_Name) const {

        if (!Address || Class_Name.empty()) return instance();

        for (instance Child : children())
        {
            if (!Child.Address) continue;
            if (Child.kind() == Class_Name) return Child;
        }

        return instance();
    }
}
