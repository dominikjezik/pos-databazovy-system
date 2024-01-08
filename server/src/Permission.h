#ifndef DATABAZOVY_SYSTEM_PERMISSION_H
#define DATABAZOVY_SYSTEM_PERMISSION_H


#include <string>
#include <stdexcept>


enum PermissionType {
    select_permission,
    insert_permission,
    update_permission,
    delete_permission
};

class Permission {
public:
    static std::string permissionTypeToString(PermissionType permission) {
        switch (permission) {
            case select_permission:
                return "select";
            case insert_permission:
                return "insert";
            case update_permission:
                return "update";
            case delete_permission:
                return "delete";
            default:
                return "";
        }
    }

    static PermissionType stringToPermissionType(std::string permission) {
        if (permission == "select") {
            return select_permission;
        } else if (permission == "insert") {
            return insert_permission;
        } else if (permission == "update") {
            return update_permission;
        } else if (permission == "delete") {
            return delete_permission;
        } else {
            throw std::invalid_argument("Neznamy typ opravnenia!");
        }
    }
};

#endif //DATABAZOVY_SYSTEM_PERMISSION_H
